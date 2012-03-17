require 'gosu/preview'

module Gosu
  module Zen
    
    @@window_args = [800, 600, {}]
    @@options = {}
    
    def window width, height, options = nil
      if $window.nil?
        @@window_args[0] = width
        @@window_args[1] = height
        @@window_args[2].merge! options if options
      else
        raise "window size can only be set before the window is created"
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
      window = ZenWindow.new *@@window_args
      @@options.each do |opt, value|
        window.send "#{opt}=", value
      end
      window.show
    end
    
    def Zen.included mod
      at_exit { run! unless $! }
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
