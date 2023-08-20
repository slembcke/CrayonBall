class BasicEffect
  attr_reader :view_node
  
	def initialize(duration)
		@duration = duration
		@ticks = 0
	end
	
	def done?
		@ticks > @duration
	end
	
	def time_value
		@ticks.to_f/(@duration - 1).to_f
	end
	
	def tick
		@ticks += 1
	end
end

UNIT_NODE_LOADER = AS::LazyLoader.new{
	Photon::Node::VAR.ad_hoc(GL::QUADS, 2, 0, 2){|var|
		var.texcoord(0, 0); var.vertex(-1, -1)
		var.texcoord(1, 0); var.vertex( 1, -1)
		var.texcoord(1, 1); var.vertex( 1,  1)
		var.texcoord(0, 1); var.vertex(-1,  1)
	}
}

class Notification < BasicEffect

  Lifespan = 250
  Raise = 12.0
  Height = 50.0
  
  attr_reader :active, :view_node
  
  def initialize(value)
  	super(Lifespan)
		@value = value
  	@color = Photon::Color::Black.clone

		scale = 0.1
    text_node = AS::QuickTextNode.new(AS::ResourceCache.load(:font, 'gfx/font'), value.to_s, 50, Height, scale, @color, true)
		outline_node = AS::QuickTextNode.new(AS::ResourceCache.load(:font, 'gfx/font_outline'), value.to_s, 50, Height, scale*2.69, Photon::Color::White, true)

		@view_node = Photon::Node::Array.new([		
			outline_node,
			text_node,
		])

		@original_y = Height
    # set initial attributes
    update()
  end
    
  def update
		t = 1.0 - time_value
	 	@view_node.children[0].color = Photon::Color.new(t, t, t, t)
    @view_node.children[0].y = @original_y + (Raise * time_value)

	 	@view_node.children[1].color = Photon::Color.new(0, 0, 0, t)
    @view_node.children[1].y = @original_y + (Raise * time_value)

  end
  
end


class ExpandEffect < BasicEffect
	def initialize(icon_ball, ticks=30, scale=3, ramp=lambda{|t| t})
		super(ticks)
		@scale, @ramp = scale, ramp
		
		tex = AS::ResourceCache.load(icon_ball.texture_name){|key| Photon::Texture::MIP.new(key)}
		@original_color = icon_ball.photon_color
		@color = @original_color.clone
		
		pos = icon_ball.body.pos
		scale = icon_ball.view.scale
		matrix = Photon::Matrix.new.translate!(pos.x, pos.y).scale!(scale, scale).rotate!(icon_ball.body.a)
		@view_node = Photon::Node::Matrix.construct(GL::MODELVIEW, matrix){
			@scale_matrix = Photon::Matrix.new
			Photon::Node::Matrix.construct(GL::MODELVIEW, @scale_matrix){
				state_hash = {:color => @color, :blend => Photon::Blend::PremultAlpha, :texture => tex}
				Photon::Node::RenderState.new(state_hash, UNIT_NODE_LOADER.load)
			}
		}
	end
	
	def update
		t = time_value
		
		ramped_t = @ramp.call(t)
		scale = ramped_t*@scale + (1 - ramped_t)
		@scale_matrix.reset!.scale!(scale, scale)
		
		c = @original_color
		t2 = 1 - t**3
		@color.color = Photon::Color.new(c.r*t2, c.g*t2, c.b*t2, 1 - t)
	end
end

class ScoreFloaty < BasicEffect
  Raise = 5.0
  
  def initialize(pos, value)
  	super(120)

  	@color = Photon::Color::Black.clone
		@original_y = pos.y
		
		font = AS::ResourceCache.load(:font, 'gfx/font')
    @view_node = AS::QuickTextNode.new(font, value.to_s, pos.x, pos.y, 0.03, @color, true)
  end
  
  def update
		t = time_value
		
  	@view_node.color = Photon::Color.new(0, 0, 0, 1 - t)
    @view_node.y = @original_y + (Raise * t)
  end
end

class WhirlwindEffect < BasicEffect
	SHARED_NODE_LOADER = AS::LazyLoader.new{
		tex = AS::ResourceCache.load('gfx/wind.png'){|key|
			Photon::Texture::MIP.new(key, :mip_smooth, :rgba, true)
		}
		
		state_hash = {:blend => Photon::Blend::PremultAlpha, :texture => tex}
		Photon::Node::RenderState.construct(state_hash){
			quads = 64
			Photon::Node::VAR.ad_hoc(GL::QUAD_STRIP, 2, 4, 2){|var|
				quads.times{|i|
					y = i.to_f/quads.to_f
					
					v = 1.0 - (2.0*y - 1.0).abs**2.7
					var.color(0, 0, 0, v)
					
					off = Math.sin(y*20.0)*0.1
					scale = y*0.6 + 0.4
					var.texcoord(0.0, y); var.vertex(-1.0*scale + off, y)
					var.texcoord(1.0, y); var.vertex( 1.0*scale + off, y)
				}
			}
		}
	}
	
	def initialize(pos, board)
		super(180)
		
		@pos, @board = pos, board
		
		@texture_matrix = Photon::Matrix.new
		@view_node = Photon::Node::Matrix.construct(GL::TEXTURE, @texture_matrix){
			matrix = Photon::Matrix.new.translate!(pos.x, pos.y).scale!(40, 40)
			Photon::Node::Matrix.new(GL::MODELVIEW, matrix, SHARED_NODE_LOADER.load)
		}
	end
	
	def update
		@texture_matrix.reset!.translate!(0, -2*time_value)
		@board.balls.each do|ball|
			body = ball.body
			
			pos = body.p
			next if @pos.y > pos.y
			
			x_delta = pos.x - @pos.x
			y_delta = pos.y - @pos.y
			
			f_up = 200
			f_sep = -x_delta*(y_delta*0.1)**2.0
			f_wave = Math.sin(y_delta*0.1 + time_value*6)*500
			
			x_damp = (x_delta.abs*0.025)**4.0
			y_damp = (y_delta)**2.0
			coef = 1.0/(x_damp*y_damp + 1.0)
			
			body.f = vec2(f_sep + f_wave, f_up)*coef
		end
	end
end

class LightningEffect < BasicEffect
	SHARED_NODE_LOADER = AS::LazyLoader.new{
		tex = AS::ResourceCache.load('gfx/lightning.png')
		
		Photon::Node::CopyRect.new(tex, 0, 0, tex.w, tex.h, -5, -3, 15, 75)
	}
	
	def initialize(ball)
		super(60)
		
		@original_color = ball.photon_color.clone
		@color = @original_color.clone
		
		pos = ball.body.pos
		state_hash = {:color => @color, :blend => Photon::Blend::PremultAlpha}
		@view_node = Photon::Node::Matrix.construct(GL::MODELVIEW, Photon::Matrix.new.translate!(pos.x, pos.y)){
			Photon::Node::RenderState.new(state_hash, SHARED_NODE_LOADER.load)
		}
	end
	
	def update
		t = time_value
		
		c = @original_color
		t2 = 1 - t**3
		@color.color = Photon::Color.new(c.r*t2, c.g*t2, c.b*t2, 1 - t)
	end
end

class BasicParticleEffect < BasicEffect
	def rand_norm
		loop do
			v = vec2(rand*2 - 1, rand*2 - 1)
			return v if v.lengthsq < 1
		end
	end
	
	GRAVITY = vec2(0, -10)
	
	def texture_name
		'gfx/particle.png'
	end
	
	def initialize(pos, scale, color)
		super(30)
		
		@orig_color = color.clone
		@color = @orig_color.clone
		
		tex = AS::ResourceCache.load(texture_name){|key|
			Photon::Texture::MIP.new(key)
		}
		
		@particle_node = Photon::Node::Matrix.construct(GL::MODELVIEW, Photon::Matrix.new){
			state_hash = {:color => @color, :blend => Photon::Blend::PremultAlpha, :texture => tex}
			Photon::Node::RenderState.new(state_hash, UNIT_NODE_LOADER.load)
		}
		
		@trajectories = Array.new(num_particles){rand_norm}
		
		matrix = Photon::Matrix.new.translate!(pos.x, pos.y).scale!(scale, scale)
		@view_node = Photon::Node::Matrix.construct(GL::MODELVIEW, matrix){
			Photon::Node::Map.new(@trajectories){|elt|
				t = time_value
				x, y = *(elt*(2 + t*15) + GRAVITY*t*t)
				
				@particle_node.matrix.reset!.translate!(x, y)
				@particle_node
			}
		}
	end
	
	def update
		t = 1 -time_value
		
		c = @orig_color
		@color.color = Photon::Color.new(c.r*t, c.g*t, c.b*t, t)
	end
end

class MenuParticleEffect < BasicParticleEffect
	def initialize(pos, color)
		super(pos, 15, color)
	end
	
	def num_particles
		50
	end
end

class ParticleEffect < BasicParticleEffect
	def initialize(ball)
		super(ball.body.pos, 2, ball.photon_color)
	end
	
	def num_particles
		25
	end
end

class BoomEffect < BasicEffect
	def rand_norm
		loop do
			v = vec2(rand*2 - 1, rand*2 - 1)
			return v if v.lengthsq < 1
		end
	end
	
	def texture_name
		'gfx/explosion.png'
	end
	
	def initialize(pos)
		super(60)
		
		scale = 15
		color = Photon::Color::White
		
		@orig_color = color.clone
		@color = @orig_color.clone
		
		tex = AS::ResourceCache.load(texture_name){|key|
			Photon::Texture::MIP.new(key)
		}
		
		@particle_node = Photon::Node::Matrix.construct(GL::MODELVIEW, Photon::Matrix.new){
			state_hash = {:color => @color, :blend => Photon::Blend::PremultAlpha, :texture => tex}
			Photon::Node::RenderState.new(state_hash, UNIT_NODE_LOADER.load)
		}
		
		@trajectories = Array.new(num_particles){[rand_norm, (rand*2.0 - 1.0)*3.0]}
		
		matrix = Photon::Matrix.new.translate!(pos.x, pos.y).scale!(scale, scale)
		@view_node = Photon::Node::Matrix.construct(GL::MODELVIEW, matrix){
			Photon::Node::Map.new(@trajectories){|trajectory, rotation|
				t = time_value
				x, y = *(trajectory*(t))
				
				scl = 1.0 - t*0.6
				@particle_node.matrix.reset!.sprite!(x, y, scl, scl, rotation*t)
				@particle_node
			}
		}
	end
	
	def num_particles
		5
	end
	
	def update
		t = 1 -time_value
		
		c = @orig_color
		@color.color = Photon::Color.new(c.r*t, c.g*t, c.b*t, t)
	end
end
