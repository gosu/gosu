require 'gosu/preview'

module Gosu
  module Zen
    
    @@video_mode = [800, 600, {}]
    @@options = {}
    
    def video width, height, options = nil
      if $window.nil?
        puts @@video_mode.inspect
        @@video_mode[0] = width
        @@video_mode[1] = height
        @@video_mode[2].merge! options
      else
        raise "video mode can only be set before the window is created"
      end
    end
    
    def set what, value
      if $window.nil?
        @@options[what.to_sym] = value
      else
        $window.send "#{what}=", value
      end
    end
    
    def button_down id = nil, &body
      m = id ? "button_down_#{id}" : :button_down_other
      ZenWindow.send :define_method, m, &body
    end
    
    def button_up id = nil, &body
      m = id ? "button_up_#{id}" : :button_up_other
      ZenWindow.send :define_method, m, &body
    end
    
    def update &body
      ZenWindow.send :define_method, :update, &body
    end
    
    def draw &body
      ZenWindow.send :define_method, :draw, &body
    end
    
    def run!
      window = ZenWindow.new *@@video_mode
      @@options.each do |opt, value|
        window.send "#{opt}=", value
      end
      window.show
    end
    
    def Zen.included mod
      at_exit &method(:run!)
    end
    
  end
  
  class ZenWindow < Window
    
    def button_down id
      m = :"button_down_#{id}"
      respond_to?(m) ? send(m) : button_down_other(id)
    end
    
    def button_up id
      m = :"button_up_#{id}"
      respond_to?(m) ? send(m) : button_up_other(id)
    end
    
    def button_down_other id
    end
    
    def button_up_other id
    end
    
  end
end
