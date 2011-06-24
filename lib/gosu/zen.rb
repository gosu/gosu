require 'gosu'
require 'singleton'

module Gosu
  class ZenWindow < Window
    include Singleton
    
    def initialize
      super 800, 600, false
    end
  end
end

def set what, value
  Gosu::ZenWindow.instance.send "#{what}=", value
end

def update(&proc)
  Gosu::ZenWindow.send :define_method, :update, proc
end

at_exit do
  Gosu::ZenWindow.instance.show
end
