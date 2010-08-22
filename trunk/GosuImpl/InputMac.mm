#import <AppKit/AppKit.h>
#import <Carbon/Carbon.h>
#include <Gosu/Input.hpp>
#include <Gosu/TextInput.hpp>
#include <GosuImpl/MacUtility.hpp>
#include <Gosu/Utility.hpp>
#include <IOKit/hidsystem/IOLLEvent.h>
#include <boost/array.hpp>
#include <boost/cstdint.hpp>
#include <map>
#include <string>
#include <vector>

#include <mach/mach.h>
#include <mach/mach_error.h>
#include <Kernel/IOKit/hidsystem/IOHIDUsageTables.h>
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/hid/IOHIDLib.h>
#include <IOKit/hid/IOHIDKeys.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/IOCFPlugIn.h>
#include <stdexcept>
#include <vector>
#include <boost/shared_ptr.hpp>

// USB Gamepad code, likely to be moved somewhere else later.
// This is Frankencode until the Input redesign happens.
namespace {
    using namespace std;
    using namespace Gosu;
    using boost::shared_ptr;
    
    template<typename Negatable>
    void checkTrue(Negatable cond, const char* message = "work")
    {
        if (!cond)
            throw runtime_error(string("HID system failed to ") + message);
    }

    void checkIO(IOReturn val, const char* message = "work")
    {
        checkTrue(val == kIOReturnSuccess, message);
    }
    
    class IOScope : boost::noncopyable
    {
        io_object_t ref;
    public:
        IOScope(io_object_t ref)
        :   ref(ref)
        {
        }
        
        ~IOScope()
        {
            IOObjectRelease(ref);
        }
    };

    string getDictString(CFMutableDictionaryRef dict, CFStringRef key, const char* what)
    {
        char buf[256];
        CFStringRef str = 
            (CFStringRef)CFDictionaryGetValue(dict, key);
        checkTrue(str && CFStringGetCString(str, buf, sizeof buf, CFStringGetSystemEncoding()),
            what);
        return buf;
    }

    SInt32 getDictSInt32(CFMutableDictionaryRef dict, CFStringRef key, const char* what)
    {
        SInt32 value;
        CFNumberRef number =
            (CFNumberRef)CFDictionaryGetValue(dict, key);
        checkTrue(number && CFNumberGetValue(number, kCFNumberSInt32Type, &value),
            what);
        return value;
    }

    struct Axis
    {
        IOHIDElementCookie cookie;
        long min, max;
        enum Role { mainX, mainY } role;

        // Some devices report more axis than they have, with random constant values.
        // An axis has to go into, and out of the neutral zone so it will be reported.
        bool wasNeutralOnce;
        
        Axis(CFMutableDictionaryRef dict, Role role)
        : role(role), wasNeutralOnce(false)
        {
            cookie = (IOHIDElementCookie)getDictSInt32(dict, CFSTR(kIOHIDElementCookieKey),
                "get an element cookie");
            min = getDictSInt32(dict, CFSTR(kIOHIDElementMinKey),
                "get a min value");
            max = getDictSInt32(dict, CFSTR(kIOHIDElementMaxKey),
                "get a max value");
        }
    };

    struct Hat
    {
        IOHIDElementCookie cookie;
        enum { fourWay, eightWay, unknown } kind;
        long min;

        Hat(CFMutableDictionaryRef dict)
        {
            cookie = (IOHIDElementCookie)getDictSInt32(dict, CFSTR(kIOHIDElementCookieKey),
                "an element cookie");
            min = getDictSInt32(dict, CFSTR(kIOHIDElementMinKey),
                "a min value");
            SInt32 max = getDictSInt32(dict, CFSTR(kIOHIDElementMaxKey),
                "a max value");
            if ((max - min) == 3)
                kind = fourWay;
            else if ((max - min) == 7)
                kind = eightWay;
            else
                kind = unknown;
        }
    };

    struct Button
    {
        IOHIDElementCookie cookie;

        Button(CFMutableDictionaryRef dict)
        {
            cookie = (IOHIDElementCookie)getDictSInt32(dict, CFSTR(kIOHIDElementCookieKey),
                "get an element cookie");
        }
    };

    struct Device
    {
        shared_ptr<IOHIDDeviceInterface*> interface;

        std::string name;
        vector<Axis> axis;
        vector<Hat> hats;
        vector<Button> buttons;
    };

    class System
    {
        vector<Device> devices;
        
        static void closeAndReleaseInterface(IOHIDDeviceInterface** ptr)
        {
            (*ptr)->close(ptr); // Won't hurt if open() wasn't called
            (*ptr)->Release(ptr);
        }
        
        static void eraseDevice(void* target, IOReturn, void* refcon, void*)
        {
            System& self = *static_cast<System*>(target);  
            for (unsigned i = 0; i < self.devices.size(); ++i)
                if (self.devices[i].interface.get() == refcon)
                {
                    self.devices.erase(self.devices.begin() + i);
                    return;
                }
            assert(false);
        }
        
        bool isDeviceInteresting(CFMutableDictionaryRef properties)
        {
            // Get usage page/usage.
            SInt32 page = getDictSInt32(properties, CFSTR(kIOHIDPrimaryUsagePageKey),
                "a usage page");
            SInt32 usage = getDictSInt32(properties, CFSTR(kIOHIDPrimaryUsageKey),
                "a usage value");
            // Device uninteresting?
            return page == kHIDPage_GenericDesktop &&
                (usage == kHIDUsage_GD_Joystick ||
                 usage == kHIDUsage_GD_GamePad ||
                 usage == kHIDUsage_GD_MultiAxisController);
        }
        
        shared_ptr<IOHIDDeviceInterface*> getDeviceInterface(io_registry_entry_t object)
        {
            IOCFPlugInInterface** intermediate = 0;
            SInt32 theScore;
            checkIO(IOCreatePlugInInterfaceForService(object, kIOHIDDeviceUserClientTypeID,
                        kIOCFPlugInInterfaceID, &intermediate, &theScore),
                        "get intermediate device interface");

            IOHIDDeviceInterface** rawResult = 0;
            HRESULT ret = (*intermediate)->QueryInterface(intermediate,
                                CFUUIDGetUUIDBytes(kIOHIDDeviceInterfaceID),
                                reinterpret_cast<void**>(&rawResult));
            (*intermediate)->Release(intermediate);
            if (ret != S_OK)
                checkTrue(false, "get real device interface through intermediate");
            
            // Yay - got it safe in here.
            shared_ptr<IOHIDDeviceInterface*> result(rawResult, closeAndReleaseInterface);
            
            checkIO((*result)->open(result.get(), 0));
            checkIO((*result)->setRemovalCallback(result.get(), eraseDevice, result.get(), this));
            
            return result;
        }
     
        static void addElement(const void* value, void* parameter)
        {
            CFMutableDictionaryRef dict = (CFMutableDictionaryRef)value;
            Device& target = *static_cast<Device*>(parameter);
            
            SInt32 elementType = getDictSInt32(dict, CFSTR(kIOHIDElementTypeKey),
                                    ("an element type on " + target.name).c_str());
            SInt32 usagePage = getDictSInt32(dict, CFSTR(kIOHIDElementUsagePageKey),
                                    ("an element page on " + target.name).c_str());
            SInt32 usage = getDictSInt32(dict, CFSTR(kIOHIDElementUsageKey),
                                    ("an element usage on " + target.name).c_str());
            
            if ((elementType == kIOHIDElementTypeInput_Misc) ||
                (elementType == kIOHIDElementTypeInput_Button) ||
                (elementType == kIOHIDElementTypeInput_Axis))
            {
                switch (usagePage)
                {
                    case kHIDPage_GenericDesktop:
                        switch (usage)
                        {
                            case kHIDUsage_GD_Y:
                            case kHIDUsage_GD_Ry:
                                target.axis.push_back(Axis(dict, Axis::mainY));
                                break;
                            case kHIDUsage_GD_X:
                            case kHIDUsage_GD_Rx:
                            case kHIDUsage_GD_Z:
                            case kHIDUsage_GD_Rz:
                            case kHIDUsage_GD_Slider:
                            case kHIDUsage_GD_Dial:
                            case kHIDUsage_GD_Wheel:
                                target.axis.push_back(Axis(dict, Axis::mainX));
                                break;
                            case kHIDUsage_GD_Hatswitch:
                                target.hats.push_back(Hat(dict));
                                break;
                        }
                        break;
                    case kHIDPage_Button:
                        target.buttons.push_back(Button(dict));
                        break;
                }
            }
            else if (elementType == kIOHIDElementTypeCollection)
                addElementCollection(target, dict);
        }
        
        static void addElementCollection(Device& target, CFMutableDictionaryRef properties)
        {
            CFArrayRef array =
                (CFArrayRef)CFDictionaryGetValue(properties, CFSTR(kIOHIDElementKey));
            checkTrue(array,
                ("get an element list for " + target.name).c_str());
            
            CFRange range = { 0, CFArrayGetCount(array) };
            CFArrayApplyFunction(array, range, addElement, &target);
        }
     
        void addDevice(io_object_t object)
        {
            // Get handle to device properties.
            CFMutableDictionaryRef properties;
            IORegistryEntryCreateCFProperties(object, &properties,
                kCFAllocatorDefault, kNilOptions);
            if (!properties)
                return;
            CFRef<> guard(properties);
            
            if (!isDeviceInteresting(properties))
                return;
            
            Device newDevice;
            newDevice.interface = getDeviceInterface(object);
            try
            {
                newDevice.name = getDictString(properties, CFSTR(kIOHIDProductKey),
                    "get a product name");
            }
            catch (const runtime_error&)
            {
                newDevice.name = "unnamed device";
            }
            addElementCollection(newDevice, properties);
            devices.push_back(newDevice);
        }
        
    public:
        System()
        {
            mach_port_t masterPort;
            checkIO(IOMasterPort(bootstrap_port, &masterPort),
                "get master port");
            
            CFMutableDictionaryRef hidDeviceKey =
                IOServiceMatching(kIOHIDDeviceKey);
            checkTrue(hidDeviceKey,
                "build device list");

            io_iterator_t iterator;
            checkIO(IOServiceGetMatchingServices(masterPort, hidDeviceKey, &iterator),
                "set up HID iterator");
            IOScope guard(iterator);
            
            while (io_registry_entry_t deviceObject = IOIteratorNext(iterator))
            {
                IOScope guard(deviceObject);
                addDevice(deviceObject);
            }
        }
        
        unsigned countDevices() const
        {
            return devices.size();
        }
        
        const Device& getDevice(unsigned i) const
        {
            return devices.at(i);
        }

        boost::array<bool, gpNum> poll()
        {
            boost::array<bool, gpNum> result;
            result.assign(false);
        
            IOHIDEventStruct event;
            for (int dev = 0; dev < devices.size(); ++dev)
            {
                // Axis
                for (int ax = 0; ax < devices[dev].axis.size(); ++ax)
                {
                    checkIO((*devices[dev].interface)->getElementValue(
                        devices[dev].interface.get(),
                        devices[dev].axis[ax].cookie, &event));
                        
                    Axis& a = devices[dev].axis[ax];
                    if (event.value < (3 * a.min + 1 * a.max) / 4.0)
                    {
                        if (a.wasNeutralOnce)
                            result[(a.role == Axis::mainX ? gpLeft : gpUp) - gpRangeBegin] = true;
                    }
                    else if (event.value > (1 * a.min + 3 * a.max) / 4.0)
                    {
                        if (a.wasNeutralOnce)
                            result[(a.role == Axis::mainX ? gpRight : gpDown) - gpRangeBegin] = true;
                    }
                    else
                        a.wasNeutralOnce = true;
                }
                
                // Hats (merge into axis)
                for (int hat = 0; hat < devices[dev].hats.size(); ++hat)
                {
                    checkIO((*devices[dev].interface)->getElementValue(
                        devices[dev].interface.get(),
                        devices[dev].hats[hat].cookie, &event));
                     
                    // In case device does not start at 0 as expected.
                    event.value -= devices[dev].hats[hat].min;
                     
                    // Treat all hats as being 8-way.
                    if (devices[dev].hats[hat].kind == Hat::fourWay)
                        event.value *= 2;

                    switch (event.value)
                    {
                        // Must...resist...doing...crappy...fallthrough...magic... 
                        case 0:
                            result[gpUp - gpRangeBegin] = true;
                            break;
                        case 1:
                            result[gpUp - gpRangeBegin] = true;
                            result[gpRight - gpRangeBegin] = true;
                            break;
                        case 2:
                            result[gpRight - gpRangeBegin] = true;
                            break;
                        case 3:
                            result[gpDown - gpRangeBegin] = true;
                            result[gpRight - gpRangeBegin] = true;
                            break;
                        case 4:
                            result[gpDown - gpRangeBegin] = true;
                            break;
                        case 5:
                            result[gpDown - gpRangeBegin] = true;
                            result[gpLeft - gpRangeBegin] = true;
                            break;
                        case 6:
                            result[gpLeft - gpRangeBegin] = true;
                            break;
                        case 7:
                            result[gpUp - gpRangeBegin] = true;
                            result[gpLeft - gpRangeBegin] = true;
                            break;
                    }
                }

                // Buttons
                for (int btn = 0; btn < devices[dev].buttons.size() && btn < 16; ++btn)
                {
                    checkIO((*devices[dev].interface)->getElementValue(
                        devices[dev].interface.get(),
                        devices[dev].buttons[btn].cookie, &event));
                    
                    if (event.value >= 1)
                        result[gpButton0 + btn - gpRangeBegin] = true;
                }
            }
            
            return result;
        }
    };
}

// Needed for char translation.
namespace Gosu
{
	std::wstring macRomanToWstring(const std::string& s);
}

namespace {
    const unsigned numScancodes = 128;
    
    boost::array<wchar_t, numScancodes> idChars;
    std::map<wchar_t, unsigned> charIds;
    
    void initCharTranslation()
    {
        static bool initializedCharData = false;
        if (initializedCharData)
            return;
        initializedCharData = true;
        
        idChars.assign(0);

#ifndef __LP64__
		const void* KCHR = reinterpret_cast<const void*>(GetScriptManagerVariable(smKCHRCache));
		if (!KCHR)
			return;
        
        for (int code = numScancodes - 1; code >= 0; --code)
        {
            UInt32 deadKeyState = 0;
			UInt32 value = KeyTranslate(KCHR, code, &deadKeyState);
			// If this key triggered a dead key, hit it again to obtain the actual value.
			if (deadKeyState != 0)
				value = KeyTranslate(KCHR, code, &deadKeyState);
			
			// No character! Pity.
			if (value == 0)
				continue;
                
            // Ignore special characters except newline.
            if (value == 3)
                value = 13; // convert Enter to Return
            if (value < 32 && value != 13)
                continue;
			
			// Now we have a character which is *not* limited to the ASCII range. To correctly
			// translate this into a wchar_t, we need to convert it based on the current locale.
			// TODO: That locale stuff should be explicit. Locales always cause trouble.
            
			std::string str(1, char(value));
			wchar_t ch = Gosu::macRomanToWstring(str).at(0);
			
            idChars[code] = ch;
            charIds[ch] = code;
        }
#endif
    }
	 
    boost::array<bool, Gosu::numButtons> buttonStates;
}

struct Gosu::Input::Impl
{
    Input& input;
    NSWindow* window;
    TextInput* textInput;
    double mouseX, mouseY;
    double mouseFactorX, mouseFactorY;
    
    unsigned currentMods;
    
    struct WaitingButton
    {
        Button btn;
        bool down;
        WaitingButton(unsigned btnId, bool down) : btn(btnId), down(down) {}
    };
    std::vector<WaitingButton> queue;

    Impl(Input& input)
    : input(input), textInput(0), mouseFactorX(1), mouseFactorY(1), currentMods(0)
    {
    }
    
    void enqueue(unsigned btnId, bool down)
    {
        queue.push_back(WaitingButton(btnId, down));
    }

    void updateMods(unsigned newMods)
    {
        static const unsigned ids[8] = { kbLeftShift, kbLeftControl,
                                         kbLeftAlt, kbLeftMeta,
                                         kbRightShift, kbRightControl,
                                         kbRightAlt, kbRightMeta };
        static const unsigned bits[8] = { NX_DEVICELSHIFTKEYMASK, NX_DEVICELCTLKEYMASK,
                                          NX_DEVICELALTKEYMASK, NX_DEVICELCMDKEYMASK,
                                          NX_DEVICERSHIFTKEYMASK, NX_DEVICERCTLKEYMASK,
                                          NX_DEVICERALTKEYMASK, NX_DEVICERCMDKEYMASK };
    
        for (unsigned i = 0; i < 8; ++i)
            if ((currentMods & bits[i]) != (newMods & bits[i]))
                enqueue(ids[i], (newMods & bits[i]) != 0);
        
        currentMods = newMods;
    }

    void refreshMousePosition()
    {
        NSPoint mousePos = [NSEvent mouseLocation];
        if (window)
        {
            mousePos = [window convertScreenToBase: mousePos];
            mousePos.y = [[window contentView] frame].size.height - mousePos.y;
        }
        else
            mousePos.y = CGDisplayBounds(CGMainDisplayID()).size.height - mousePos.y;
        mouseX = mousePos.x;
        mouseY = mousePos.y;
    }
};

Gosu::Input::Input(void* window)
: pimpl(new Impl(*this))
{
    pimpl->window = static_cast<NSWindow*>(window);
    buttonStates.assign(false);
    initCharTranslation();
}

Gosu::Input::~Input()
{
}

bool Gosu::Input::feedNSEvent(void* event)
{
    NSEvent* ev = (NSEvent*)event;
    unsigned type = [ev type];

    if (type == NSKeyDown && textInput() && textInput()->feedNSEvent(event))
        return true;
        
    if (type == NSKeyDown && [ev isARepeat])
        return false;
            
    // Process modifier keys.
    unsigned mods = [ev modifierFlags];
    if (mods != pimpl->currentMods)
        pimpl->updateMods(mods);
    
    // Handle mouse input.
    switch (type)
    {
        case NSLeftMouseDown:
            pimpl->enqueue(msLeft, true);
            return true;
        case NSLeftMouseUp:
            pimpl->enqueue(msLeft, false);
            return true;
        case NSRightMouseDown:
            pimpl->enqueue(msRight, true);
            return true;
        case NSRightMouseUp:
            pimpl->enqueue(msRight, false);
            return true;
        case NSScrollWheel:
            if ([ev deltaY] > 0)
            {
                pimpl->enqueue(msWheelUp, true);
                pimpl->enqueue(msWheelUp, false);
            }
            else if ([ev deltaY] < 0)
            {
                pimpl->enqueue(msWheelDown, true);
                pimpl->enqueue(msWheelDown, false);
            }
            else
                return false;
            return true;
    }
	
    // Handle other keys.
    if (type == NSKeyDown || type == NSKeyUp)
    {
        pimpl->enqueue([ev keyCode], type == NSKeyDown);
        return true;
    }
	
    return false;
}

wchar_t Gosu::Input::idToChar(Button btn)
{
    if (btn.id() < numScancodes)
        return idChars[btn.id()];
    else
        return 0;
}

Gosu::Button Gosu::Input::charToId(wchar_t ch)
{
    std::map<wchar_t, unsigned>::const_iterator iter = charIds.find(ch);
    if (iter == charIds.end())
        return noButton;
    return Gosu::Button(iter->second);
}

bool Gosu::Input::down(Gosu::Button btn) const
{
    if (btn == noButton || btn.id() >= numButtons)
        return false;

    return buttonStates.at(btn.id());
}

double Gosu::Input::mouseX() const
{
    return pimpl->mouseX * pimpl->mouseFactorX;
}

double Gosu::Input::mouseY() const
{
    return pimpl->mouseY * pimpl->mouseFactorY;
}

void Gosu::Input::setMousePosition(double x, double y)
{
    NSPoint mousePos = NSMakePoint(x / pimpl->mouseFactorX, y / pimpl->mouseFactorY);
    if (pimpl->window)
    {
        mousePos.y = [[pimpl->window contentView] frame].size.height - mousePos.y;
        mousePos = [pimpl->window convertBaseToScreen: mousePos];
        mousePos.y = CGDisplayBounds(CGMainDisplayID()).size.height - mousePos.y;
    }

    CGSetLocalEventsSuppressionInterval(0.0);
    CGWarpMouseCursorPosition(CGPointMake(mousePos.x, mousePos.y));
    
    pimpl->refreshMousePosition();
}

void Gosu::Input::setMouseFactors(double factorX, double factorY)
{
    pimpl->mouseFactorX = factorX;
    pimpl->mouseFactorY = factorY;
}

const Gosu::Touches& Gosu::Input::currentTouches() const
{
    static Gosu::Touches none;
    return none;
}

double Gosu::Input::accelerometerX() const
{
    return 0.0;
}

double Gosu::Input::accelerometerY() const
{
    return 0.0;
}

double Gosu::Input::accelerometerZ() const
{
    return 0.0;
}

void Gosu::Input::update()
{
    pimpl->refreshMousePosition();
    
    for (unsigned i = 0; i < pimpl->queue.size(); ++i)
    {
        Impl::WaitingButton& wb = pimpl->queue[i];
        buttonStates.at(wb.btn.id()) = wb.down;
        if (wb.down && onButtonDown)
            onButtonDown(wb.btn);
        else if (!wb.down && onButtonUp)
            onButtonUp(wb.btn);
    }
    pimpl->queue.clear();
    
    static System sys;
    boost::array<bool, gpNum> gpState = sys.poll();
    for (unsigned i = 0; i < gpNum; ++i)
    {
        if (buttonStates[i + gpRangeBegin] != gpState[i])
        {
            buttonStates[i + gpRangeBegin] = gpState[i];
            if (gpState[i] && onButtonDown)
                onButtonDown(Button(gpRangeBegin + i));
            else if (!gpState[i] && onButtonUp)
                onButtonUp(Button(gpRangeBegin + i));
        }
    }
}

Gosu::TextInput* Gosu::Input::textInput() const
{
    return pimpl->textInput;
}

void Gosu::Input::setTextInput(TextInput* textInput)
{
    pimpl->textInput = textInput;
}

