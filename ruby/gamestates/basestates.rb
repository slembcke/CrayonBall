class BaseState < AS::GameState::TickLimitedState
	
	def initialize(delegate)
		super(1.0/60.0, delegate)
		
		@@mouse ||= Mouse.new
		@draw_mouse = (delegate.respond_to?(:draw_mouse) ? delegate.draw_mouse : true)
		
	end
	
	def enter_state
		super
		FadeState.new(self, 15).run
		reset_timing
	end
	
	#THIS IS A NASTY HACK
	# override this gamestate method to filter and mutate mouse events
	def handle_events
		@@event_source.dequeue_events{|event|
			if event.respond_to?(:pos)
				x, y = event.pos
				event.pos.replace([x = (x - @@subx)*800.0/@@virtual_w, y = (y - @@suby)*600.0/@@virtual_h])
			end
			
			input(event)
		}
	end
	
	def input(event)
		@@mouse.update_pos(*event.pos) if AS::Event::MouseMotionEvent === event
		super
	end
	
	def self.set_screen_size(w, h)
		ratio = w.to_f/h.to_f
		if ratio >= 4.0/3.0
			puts "wide ratio"
			@@virtual_w = h*4.0/3.0
			@@virtual_h = h
			@@subx = (w - @@virtual_w)/2
			@@suby = 0
		else
			puts "narrow ratio"
			@@virtual_w = w
			@@virtual_h = w*3.0/4.0
			@@subx = 0
			@@suby = (h - @@virtual_h)/2
		end
		
		pad_x = [800.0*(w - @@virtual_w)/(2.0*@@virtual_w), 800].min
		pad_y = [600.0*(h - @@virtual_h)/(2.0*@@virtual_h), 600].min
		@@screen_bounds = [-pad_x, -pad_y, 800*w/@@virtual_w, 600*h/@@virtual_h] #lbwh
		
		GL.MatrixMode(GL::PROJECTION)
		GL.LoadIdentity
		GLU.Ortho2D(-pad_x, 800 + pad_x, -pad_y, 600 + pad_y)
		
		GL.Enable(GL::SCISSOR_TEST)
		GL.Scissor(0, @@suby, w, @@virtual_h)
	end
	
	# left, bottom, width, height
	def self.screen_bounds
		@@screen_bounds
	end
	
	def draw
		super
		@@mouse.view_node.render if @draw_mouse
	end
			
end

class FadeState < AS::GameState::TickLimitedState
	
	def initialize(parent, fade_duration)
		super(1.0/60.0, nil)
		@parent = parent
		@fade_duration = fade_duration
		
		@ticks = 0
	end
	
	def enter_state
		super
		
		@color = Photon::Color.new(1.0, 1.0, 1.0, 1.0)
		
		@tex = ScreenCopyTexture.new(:smooth, :rgb)
		
		state = {:blend => Photon::Blend::Alpha, :color => @color, :texture => @tex}
		@overlay = Photon::Node::RenderState.construct(state){
			Photon::Node::CopyRect.new(@tex, 0, 0, @tex.w, @tex.h, *BaseState.screen_bounds)
		}
	end
	
	def exit_state
		@tex.release
	end
	
	def update
		super
		@parent.update
		
		@ticks += 1
		AS::GameState.stop if @ticks > @fade_duration
	end
	
	def draw
		super
		
		@parent.draw
		
		alpha = 1.0 - @ticks.to_f/@fade_duration
		@color.color = Photon::Color.new(1.0, 1.0, 1.0, alpha)
		
		@overlay.render
	end
end


class SwipeState < AS::GameState::TickLimitedState

	def initialize(parent, transition_duration)
		super(1.0/60.0, nil)
		@parent = parent
		@transition_duration = transition_duration
		
		@ticks = 0
	end
	
end

