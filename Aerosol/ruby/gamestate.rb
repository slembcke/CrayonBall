require 'ruby/aerosol.rb'

class AS::GameState
  # class accessors
  def self.event_source; @@event_source; end
  def self.event_source=(source); @@event_source = source; end
  
  attr_accessor :delegate
  
  def method_missing(name, *args)
  	#delegate this call.
  	@delegate.send(name, *args)
  end
  
  @@should_draw = true
  
  def self.should_draw=(bool)
    @@should_draw = bool
  end
  
  def initialize(delegate = nil)
    @delegate = delegate
    @input_listeners = []
  end
  
  def handle_events
    @@event_source.dequeue_events{|event| input(event)}
  end
  
  # Called as the body of the game loop
  def loop_step
    sync
    
    # poll events
    handle_events
    
    # update loop
    update while needs_update
    
    if needs_redraw and @@should_draw
    	draw
    	AS.swap_buffers
    end
  end
  
  # Start a game state loop
  def run
    # run the run loop until broken
    ret_values = catch(:gamestate_exit) do
			# run activation code
			enter_state
			
      loop do
        loop_step
      end
    end
    
    # run cleanup code
    exit_state
    
    return *ret_values
  end
  
  # Exit the current game state
  def self.stop(*values)
    throw(:gamestate_exit, values)
  end
  
  # Used to run a gamestates in a state-machine-like manner. It runs the supplied
  # game state and then uses the  state exit values as the next state to run.
  # See tests/gamestate/gamestate.rb
  def self.state_machine(state)
    state = state.run while state
  end
  
  # Perform any time syncronization calls here. Update timers, call sleep, etc.
  def sync
  	@delegate.sync if @delegate.respond_to? :sync
  end
  
  # Handle individual input events. Can be delegated.
  def input(event)
  	@delegate.input(event) if @delegate.respond_to? :input
  end
  
  # Return true if the update method should be called.
  # Can be delegated. Returns false by default.
  def needs_update
  	if @delegate.respond_to? :needs_update
  		@delegate.needs_update
  	else
	    false
	   end
  end  
  
  # Update the game logic.
  # Runs in a tight loop until needs_update returns false.
  # Can be delegated.
  def update
  	@delegate.update if @delegate.respond_to? :update
  end

	# Returns true if the draw method should be called.
	# Can be delegated. Returns true by default.
  def needs_redraw
		if @delegate.respond_to? :needs_redraw
			@delegate.needs_redraw
		else
			true
		end
  end
  
  # Draw a frame.
  # Runs once per loop.
  # Can be delegated.
  def draw
  	@delegate.draw if @delegate.respond_to? :draw
		
		# TODO need to add a debug variable that runs this.
  	#until((error = GL.GetError) == 0)
    #  puts GLU.ErrorString(error)
    #end
  end
  
  # Called when a state is activated.
  # Can be delegated.
  def enter_state
  	@delegate.enter_state if @delegate.respond_to? :enter_state
  end
  
  # Called when a state exits.
  # Can be delegated.
  def exit_state
  	@delegate.exit_state if @delegate.respond_to? :exit_state
  end
  
  class SimpleState < AS::GameState
  	attr_reader :needs_update
  	
  	def initialize(delegate)
  		super(delegate)
  		
  		@needs_update = true
  	end
  	
  	def update
  		@needs_update = false
  		super
  	end
  	
  	def draw
  		@needs_update = true
  		super
  	end
  end
  
  class TickLimitedState < AS::GameState
  	@@default_time_source = Time.method(:now)
  	
  	attr_accessor :time_source
  	attr_accessor :max_frameskip
  
  	def initialize(time_step, delegate=nil)
  		@time_step = time_step
  		super(delegate)

  		@time_source = @@default_time_source
  		@max_frameskip = 10
  	end
  	
  	def reset_timing
  		@ref_time = @time_source.call
  		@ref_ticks = 0
  	end
  	
  	def enter_state
  		# Upon entering the state, keep a reference time and a count of ticks since then.
  		reset_timing
  		super
  	end
  	
  	def sync
  		# Store the time of the current loop
  		@cur_time = @time_source.call
  		
  		# Reset ref time if it gets too far behind.
  		overtime = (@cur_time - @ref_time) - (@ref_ticks + @max_frameskip)*@time_step
  		@ref_time += overtime if overtime > 0
  		
  		super
  	end
  	
  	def needs_update
  		# Returns true until ticks*step catches up to the time since #run was called.
  		(@cur_time - @ref_time).to_f > @ref_ticks*@time_step
  	end
  	
  	def update
  		@ref_ticks += 1
  		super
  	end
  end
end
