class CursorBall < LockBall
	
	def rand_radius
		@radius
	end
	
	def rand_color
		@color
	end
	
	def initialize(pos, radius, color)
		@radius, @color = radius, color
		
		super(pos)
	end
end

class DropGameBoard < AbstractGameBoard

	MAX_DROP_SPEED = 20
	
	def initialize(*args)
		@last_queue = 0
		@overflow = 0
		@ball_factory = DropBallFactory.new
		
		# Array used to box the drop ball cursor node
		@drop_ball_box = []
		advance_drop_ball(vec2(50, BoardHeight - NormalBall.max_radius))
		
		super
	end
	
	def drop_ball=(ball)
		@drop_ball = ball
		@drop_ball_box.replace([@drop_ball.view.view_node])
	end
	
	def advance_drop_ball(pos)
		@drop_cooldown = MAX_DROP_SPEED
		@next_drop_ball = @ball_factory.next_normal
		@next_drop_ball.body.pos = pos
		
		self.drop_ball = CursorBall.new(pos, @next_drop_ball.radius, @next_drop_ball.color)
	end
	
	def make_view_node
		node = super
		
		#insert the cursor ball node right before the effects
		node.children.insert(-2, Photon::Node::Array.new(@drop_ball_box))
		
		return node
	end
	
	def move_dropper(x, y)
		point = vec2(*clamp_mouse(x, y, @drop_ball.class.max_radius))
		@drop_ball.body.pos = point
		@next_drop_ball.body.pos = point
	end

	def drop_ball(x, y)
		if CursorBall === @drop_ball
			@drop_ball.clicked_on(self) # plays animation and sound 
			return
		end
		
		x, y = clamp_mouse(x, y, @drop_ball.class.max_radius)
		pos = vec2(x, y)
		
		@drop_ball.body.vel = vec2(0,-80)
		@drop_ball.body.pos = pos
		add_ball(@drop_ball)
		
		advance_drop_ball(pos)
#		self.drop_ball = @ball_factory.next_normal
#		@drop_ball.body.pos = vec2(x, y)
#		@drop_cooldown = MAX_DROP_SPEED
	end

	def group_cleared(group)
		super
		
		@overflow -= group.length
	end
	
	def update
		self.drop_ball = @next_drop_ball if @drop_cooldown == 0
		
		@drop_cooldown -= 1 if !super
		balls = @balls.length + @ball_queue.length
		n = 4
		
		ballDiff = (130-balls)
		ballDiff = 1 if ballDiff < 1
		nextTime = ((120*n)/ballDiff) - n
		if @last_queue + nextTime < @ticks
			@last_queue = @ticks
			@ball_queue << @ball_factory.next_ball(@ticks)
		end
		
		@overflow = 0 if @overflow < 0
		@gamestate_delegate.game_over if @overflow > 120 * 2
		
		@bar.bar = @overflow / 2

		rate = (12.0*1.4**(-@ticks/3600.0)).to_i
		@overflow += 1 if @ticks%rate == 0
	end
end

class DropBallFactory
 
  TheseBalls = [BombBall, StormBall]
  Width = 100
	Height = 75

	def next_ball(ticks)
		mult = (ticks / 21600.0) 
		mult = 1.0 if mult > 1.0
		if rand < 0.07 + (0.07 * mult)
			return random_ball(TheseBalls[rand(TheseBalls.length)])
		else
			return random_ball(NormalBall)
		end
	end
	
	def next_normal
		random_ball(NormalBall)
	end
 
 	def random_ball(ball_class)
		radius = ball_class.max_radius
 		pos = vec2(radius + rand(Width - 2*radius), 1.5*Height + radius)
		
		ball_class.new(pos)
	end
end

class DropGameStateDelegate < AbstractGameStateDelegate


	def initialize
		super(DropGameBoard.new(self))
		init_input
	end
		
		
	def draw_mouse
		false
	end
	
	def mode
		:drop
	end
	
	def init_input
		
		@action_listener.action(AS::Event.source_for_mbutton(1)){|event|
			next unless event.state #ignore button up events
			
			coord = mouse_to_board(*event.pos)
			@board.drop_ball(coord.x, coord.y)
		}
		
		@action_listener.action(AS::Event.source_for_mouse){|event|
			coord = mouse_to_board(*event.pos)
			@board.move_dropper(coord.x, coord.y)
		}
	end

end
