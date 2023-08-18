# Convenient event handler.  Once controls are defined, their values
# can be read like a hash.  Controls can be stacked so a joystick axis
# or key pair axis can use the same control name. To trigger an action,
# use the provided call method, passing it the event that occured. The
# appropriate code will be executed.
class AS::Event::InputPad
  # Set the dead zone for joystick axis used with this pad.
  attr_accessor :dead_zone
  
  # Access the actions hash. This contains blocks of code that
  # are keyed to their source, like a keyboard button.
  attr_reader :actions

  def initialize
    reset
    @dead_zone = 0.0
  end
  
  # Remove all defined controls and actions
  def reset
    @controls = {}
    @actions = {}
    @values = {}
  end

  # Retrieve the value of a control.
  def [](name)
    @values[name]
  end

  # Set the value of a control.
  def []=(name, value)
    @values[name] = value
    
    proc = @actions[name]
    proc.call(value) if proc
  end

  # Pass an event.
  def call(event)
    proc = @controls[event.source_id]
    proc.call(event) if proc
  end
  
  # Add an action to this pad to be invoked when a control is updated.
  def action(name, &proc)
    @actions[name] = proc
  end
  
  # Add a button control to the pad.  Event can be any key or button
  # event.
  def button(name, source_id, &proc)
    @values[name] = false
    @controls[source_id] = lambda{|event| self[name] = event.state}
    action(name, &proc) if proc
  end

  # Add a button control bound to a key.
  def key(name, key, &proc)
    button(name, AS::Event.source_for_key(key), &proc)
  end
  
  # Add an axis control bound to a pair of buttons or keys. (digital
  # axis)
  def daxis(name, source_id1, source_id2)
    self[name] = 0.0
    
    @controls[source_id1] = lambda{|event|
      if event.state
        self[name] = -1.0
      elsif self[name] == -1.0
        self[name] = 0.0
      end
    }
    
    @controls[source_id2] = lambda{|event|
      if event.state
        self[name] = 1.0
      elsif self[name] == 1.0
        self[name] = 0.0
      end
    }
  end
  
  # Add an axis control bound to a pair of keys. (key axis)
  def kaxis(name, key1, key2)
    daxis(name, AS::Event.source_for_key(key1), AS::Event.source_for_key(key2))
  end
end
