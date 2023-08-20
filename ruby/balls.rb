class AbstractBall
	include CP::Object
	
	BallClasses = []
	
	attr_reader :view, :body, :color, :radius
	
	# colors are now just integer indexes
	Colors = (0...6).to_a
	WILDCARD_COLOR = 6
	
	RealColors = [
		Photon::Color::Red,
		Photon::Color.new(1, 0.5, 0),
		Photon::Color::Yellow,
		Photon::Color::Magenta,
		Photon::Color::Green,
		Photon::Color::Blue,
		Photon::Color::White,
	]
	
	def photon_color
		RealColors[@color]
	end
	
#	Colors = [
#		Photon::Color.new(237/255.0, 46/255.0, 56/255.0),
#		Photon::Color.new(89/255.0, 181/255.0, 72/255.0),
#		Photon::Color.new(0/255.0, 129/255.0, 199/255.0),
#		Photon::Color.new(255/255.0, 230/255.0, 23/255.0),
#		Photon::Color.new(250/255.0, 128/255.0, 61/255.0),
#		Photon::Color.new(193/255.0, 81/255.0, 184/255.0),
#	]
	
	UNCOLLIDED_COLLISION_TYPE = Object.new
	
	class Shape < CP::Shape::Circle
		attr_reader :ball
		
		def initialize(ball, color, body, radius)
			@ball = ball

			super(body, radius, vec2(0,0))
			self.u = 0.4
			self.collision_type = UNCOLLIDED_COLLISION_TYPE
		end
	end
	
	class View
		attr_reader :view_node, :scale
			
		# scale factor to offset the padding on the texture
		Scale = 256.0/240.0
		
		# Return the base view object
		# base texture, or base texture + icon, etc
		def view_base(color)
			ThemeManager.ball_node_for_color(color)
		end
		
		def initialize(body, color, radius)
			@scale = radius*Scale
			scale_matrix = Photon::Matrix.new.scale!(@scale, @scale)
			
			@view_node = Photon::Node::Array.new([
				# node for the base ball texture
				Photon::Node::Body.construct(body){
					Photon::Node::Matrix.new(GL::MODELVIEW, scale_matrix, view_base(color))
				},
				# node for the shading
				Photon::Node::BodyPos.construct(body){
					Photon::Node::Matrix.construct(GL::MODELVIEW, scale_matrix){
						# the shading node is the last in the shared nodes
						ThemeManager.wildcard_ball_node
					}
				}
			])
		end
	end
	
	def self.max_radius
		5
	end
	
	def self.rand_radius
		max_radius - rand*2
	end
	
	def mass(radius)
		2.0*Math::PI*@radius**2.0/100.0
	end
	
	def rand_radius
		self.class.rand_radius
	end
	
	def initialize(pos, color)
		@color = color
		@radius = rand_radius
		
		mass = mass(@radius)
		moment = CP.moment_for_circle(mass, @radius, 0, vec2(0,0))
		
		@body = CP::Body.new(mass, moment)
		@body.pos = pos
		@body.vel = vec2(50*(rand*2 - 1), 0)
		
		@shape = Shape.new(self, @color, @body, @radius)
		@view = make_view_node
		
		init_chipmunk_object(@body, @shape)
	end
	
	def rand_color
		Colors[rand(Colors.length)]
	end
	
	def make_view_node
		View.new(@body, @color, @radius)
	end
	
	# return additional balls to be removed when this ball is matched in a group
	def expand_group(board)
		[]
	end
	
	# Return the special sound that this ball should play when it's removed
	def special_sound
		nil
	end

	# Called when a ball is clicked on
	# should return if the ball can be removed or not
	def clicked_on(board)
		# TODO
		AS::ResourceCache.load('sfx/pop.ogg').play(2.0**((rand(5) - 2)/8.0))
		return true
	end
	
	# Returns true if pos within the ball's radius + proximity_radius
	def selected?(pos, proximity_radius=0)
		@body.pos.near?(pos, @radius + proximity_radius)
	end
end

class NormalBall < AbstractBall
	def initialize(pos, color = rand_color)
		super(pos, color)
	end
end

class WildcardBall < AbstractBall
	def initialize(pos, ignore_me = nil)
		super(pos, WILDCARD_COLOR)
	end
	
	def clicked_on(board)
		board.match_wildcard(self)
		Stats[:wildcardsClicked]+=1
		return false
	end

	AbstractBall::BallClasses << self
end

class IconBall < AbstractBall
	
	class IconView < AbstractBall::View
		
		def initialize(body, color, radius, texture_name)
			@texture_name = texture_name
			super(body, color, radius)
		end
		
		UNIT_NODE_LOADER = AS::LazyLoader.new{
			Photon::Node::VAR.ad_hoc(GL::QUADS, 2, 0, 2){|var|
				var.texcoord(0, 0); var.vertex(-1, -1)
				var.texcoord(1, 0); var.vertex( 1, -1)
				var.texcoord(1, 1); var.vertex( 1,  1)
				var.texcoord(0, 1); var.vertex(-1,  1)
			}
		}
		
		# override view base to insert an icon texture over the base ball texture
		def view_base(color)
			texture = AS::ResourceCache.load(@texture_name){|key|
				Photon::Texture::MIP.new(key)
			}
			
			state_hash = {:texture => texture, :blend => Photon::Blend::PremultAlpha}
			
			Photon::Node::Array.new([
				super(color),
				Photon::Node::RenderState.construct(state_hash){
					UNIT_NODE_LOADER.load
				}
			])
		end
	end 
	

	def initialize(pos, color = rand_color)
		super(pos, color)
	end
	
	# overriden to provide texture_name to the IconView class
	def make_view_node
		IconView.new(@body, @color, @radius, texture_name)
	end

end

class LockBall < IconBall
	def texture_name
		'gfx/lockpowerup.png'
	end

	def clicked_on(board)
		AS::ResourceCache.load('sfx/clunk.ogg').play(2.0**((rand(3) - 1)/8.0))
		board.add_effect(ExpandEffect.new(self))
		
		return false
	end
	
	AbstractBall::BallClasses << self
end


class PushBall < IconBall
	def texture_name
		'gfx/jumppowerup.png'
	end

	def clicked_on(board)
		radius = 10
		v_mag = 100		
		w_mag = 10
		
		balls = board.balls_near(self.body.p, radius)
		balls.each{|ball| ball.body.v += body.rot*v_mag}
		self.body.w += w_mag
		
		Stats[:pushBalls]+=1
		AS::ResourceCache.load('sfx/spring.ogg').play
		board.add_effect(ExpandEffect.new(self))

		return false
	end
	
	AbstractBall::BallClasses << self
end

class BombBall < IconBall

	def initialize(pos, color = rand_color)
		@v_mag = 30
		super(pos, color)
	end

	def texture_name
		'gfx/bombpowerup.png'
	end

	def expand_group(board)
		Stats[:bombsExploded]+=1
		board.add_effect(BoomEffect.new(self.body.pos))

		radius = 15
		return board.balls_near(self.body.p, radius)
	end
	
	def special_sound
		AS::ResourceCache.load('sfx/boom.ogg').play
	end
	
	def set_magnitude(v)
		@v_mag = v
	end

	def clicked_on(board)
		radius = 30
		
		balls = board.balls_near(self.body.p, radius)
		balls.each{|ball| 
			next if ball == self
			
			delta = ball.body.p - self.body.p
			delta_len = delta.length
			mag = (radius - delta_len)/(delta_len**2)*@v_mag
			
			ball.body.v += delta*mag
		}
		
		Stats[:bombsPopped]+=1
		AS::ResourceCache.load('sfx/boom.ogg').play
		board.add_effect(ExpandEffect.new(self, 10, 2, lambda{|t| 2 - 2**(-1*t)}))
		
		return true
	end
	
	AbstractBall::BallClasses << self
end

class StormBall < IconBall
	def texture_name
		'gfx/stormpowerup.png'
	end

	LIGHTNING_WIDTH = 15
	
	def expand_group(board)
		board.add_effect(LightningEffect.new(self))
		Stats[:stormLightning]+=1
   
		x, y = *(body.p)
		# find all the pieces that were hit by the lightning
		return board.balls.select do|ball|
			(ball.body.p.x - x).abs < LIGHTNING_WIDTH and ball.body.p.y > y
		end
	end
	
	def special_sound
		AS::ResourceCache.load('sfx/lightning.ogg').play
	end

	def clicked_on(board)
		# Trigger a whirlwind effect.
		Stats[:stormWind]+=1
		
		board.add_effect(WhirlwindEffect.new(self.body.pos, board))
		board.add_effect(ExpandEffect.new(self))
		AS::ResourceCache.load('sfx/windstorm.ogg').play
		
		return true
	end
	
	AbstractBall::BallClasses << self
end
