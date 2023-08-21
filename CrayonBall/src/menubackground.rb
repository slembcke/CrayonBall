def moment_for_segment(mass, a, b)
	c = (a + b)/2
	delta = a - b
	l = delta.length
	
	return mass*(l**2.0/12.0 + c.lengthsq)
end

class FlipFlop

		LAZY_NODE = AS::LazyLoader.new {
			tex = AS::ResourceCache.load('gfx/menu/flip_flop.png')
			Photon::Node::SpriteTemplate.new(tex, 27, 8)
		}

	def initialize(pos)
		l_bottom = 48
		l_top = 24
		
		cg_y = (l_top**2 * 0.5)/(l_bottom + l_top)
		
		bottom = [
			vec2( l_bottom/2.0, -cg_y),
			vec2(-l_bottom/2.0, -cg_y),
		]
		
		top = [
			vec2(0, -cg_y),
			vec2(0, l_top - cg_y),
		]
		
		moment = moment_for_segment(1, *bottom) + moment_for_segment(1, *top)
		@body = CP::Body.new(1, moment)
		@body.p = pos + vec2(0, cg_y)
		@body.w = 1.0
		#space.add_body(body)
		
		a, b = *bottom
		@shape1 = CP::Shape::Segment.new(@body, a, b, 0)
		@shape1.u = 1.0
		#space.add_shape(shape)
		
		a, b = *top
		@shape2 = CP::Shape::Segment.new(@body, a, b, 0)
		@shape2.u = 1.0
		#space.add_shape(shape)
		
		@joint1 = CP::Constraint::PivotJoint.new(@body, STATIC_BODY, vec2(0, -cg_y), pos)
		@joint2 = CP::Constraint::SlideJoint.new(@body, STATIC_BODY, vec2(0, 80), pos + vec2(0, 80), 0, 56)
		
		
		@node = Photon::Node::Body.new(@body, LAZY_NODE.load)
	end
	
	def add_to_world(world)
		space = world.space
		
		space.add_body(@body)
		space.add_shape(@shape1)
		space.add_shape(@shape2)
		space.add_constraint(@joint1)
		space.add_constraint(@joint2)
		
		world.bits_node.children << @node
	end
end

class Spinner
	LAZY_NODE = AS::LazyLoader.new {
		tex = AS::ResourceCache.load('gfx/menu/spinner.png')
		Photon::Node::SpriteTemplate.new(tex, tex.w/2.0, tex.h/2.0)
	}
	
	def initialize(pos, w)
		@body = CP::Body.new(Float::INFINITY, Float::INFINITY)
		@body.p = pos
		@body.w = w
		
		l = 48
		@shape1 = CP::Shape::Segment.new(@body, vec2(-l,0), vec2(l,0), 0)
		@shape1.u = 1
		
		@shape2 = CP::Shape::Segment.new(@body, vec2(0,-l), vec2(0,l), 0)
		@shape2.u = 1
		
		@node = Photon::Node::Body.new(@body, LAZY_NODE.load)
	end
	
	def update(dt)
		@body.a += @body.w*dt
	end
	
	def add_to_world(world)
		space = world.space
		
		space.add_shape(@shape1)
		space.add_shape(@shape2)
		
		world.bits_node.children << @node
	end
end

class Tipper
	
	LAZY_NODE = AS::LazyLoader.new {
		tex = AS::ResourceCache.load('gfx/menu/tipper.png')
		Photon::Node::SpriteTemplate.new(tex, 5, 5)
	}
	
	def initialize(pos)
		mass = 4
		@body = CP::Body.new(mass, mass*192)
		@body.p = pos - vec2(48, 0)
		#space.add_body(body)
		
		@shape1 = CP::Shape::Segment.new(@body, vec2(0, 0), vec2(192, 0), 0)
		@shape1.u = 1
		#space.add_shape(shape)
		
		@shape2 = CP::Shape::Segment.new(@body, vec2(0, 0), vec2(0, 48), 0)
		@shape2.u = 1
		#space.add_shape(shape)
		
		@shape3 = CP::Shape::Segment.new(@body, vec2(48, 48), vec2(96, 96), 0)
		@shape3.u = 0
		#space.add_shape(shape)
		
		@joint1 = CP::Constraint::PivotJoint.new(@body, STATIC_BODY, vec2(48, 0), pos)
		@joint2 = CP::Constraint::SlideJoint.new(@body, STATIC_BODY, vec2(0, 0), pos - vec2(48, 0), 0, 24)

		@node = Photon::Node::Body.new(@body, LAZY_NODE.load)
	end
	
	def add_to_world(world)
		space = world.space
		
		space.add_body(@body)
		space.add_shape(@shape1)
		space.add_shape(@shape2)
		space.add_shape(@shape3)
		space.add_constraint(@joint1)
		space.add_constraint(@joint2)
		
		world.bits_node.children << @node
	end
end

class Flipper

	FlipMatrix = Photon::Matrix.new.scale!(-1 , 1)
	
	LAZY_NODE = AS::LazyLoader.new {
		tex = AS::ResourceCache.load('gfx/menu/flipper.png')
		Photon::Node::SpriteTemplate.new(tex, 23, 20)
	}
	
	def flip(node)
		return Photon::Node::Matrix.new(GL::MODELVIEW, FlipMatrix, node)
	end

	def initialize(pos, right)
		@body = CP::Body.new(1, 128)
		@body.p = pos - vec2(0, 16)
		#space.add_body(body)
		
		l = 48*(right ? -1 : 1) # segment length
		v = vec2(l, 16)
		@shape = CP::Shape::Segment.new(@body, vec2(0, 16), v, 0)
		@shape.u = 1
		#space.add_shape(shape)
		
		@joint1 = CP::Constraint::PivotJoint.new(@body, STATIC_BODY, @body.world2local(pos), pos)
		@joint2 = CP::Constraint::SlideJoint.new(@body, STATIC_BODY, v, pos + vec2(l, -48), 0, 48)
		
		node = LAZY_NODE.load
		@node = Photon::Node::Body.new(@body, (right ? flip(node) : node))
	end
	
	def add_to_world(world)
		space = world.space
		
		space.add_body(@body)
		space.add_shape(@shape)
		space.add_constraint(@joint1)
		space.add_constraint(@joint2)
		
		world.bits_node.children << @node
	end
end

STATIC_BODY = CP::Body.new(Float::INFINITY, Float::INFINITY)

class MenuBall < NormalBall
	
	def self.rand_radius
		24
	end
	
	def self.max_radius
		24
	end
	
end

class MenuBackground
	attr_reader :view_root, :balls_node, :bits_node, :space
	
	def self.instance
		@instance = MenuBackground.new if !@instance
		return @instance
	end
	
	def initialize
		@space = CP::Space.new
		@space.iterations = 15
		@space.gravity = vec2(0, -800)
		
		@ticks = 0
		@updatables = []
		@balls = []
		tex = AS::ResourceCache.load('gfx/menu/back.png')
		
		v = 0.7
		blend_color = Photon::Color.new(v,v,v,v)
		transparentLayer = Photon::Node::RenderState.construct({:color => blend_color, :blend => Photon::Blend::PremultAlpha}) do
			Photon::Node::VAR.ad_hoc(GL::QUADS, 2, 0, 0) { | var|
				var.vertex(-80, 600)
				var.vertex(-80,   0)
				var.vertex(880,   0)
				var.vertex(880, 600)
			}
		end
		
		@view_root = Photon::Node::Array.new [
			Photon::Node::RenderState.construct(:texture => tex) {
				Photon::Node::CopyRect.new(tex, 0, 0, tex.w, tex.h, -80, 0, 960, 600)
			},
			(@balls_node = Photon::Node::Array.new([])),
			(Photon::Node::RenderState.construct({:blend => Photon::Blend::PremultAlpha}, @bits_node = Photon::Node::Array.new([]))),
			transparentLayer,		
		]
			
		# feeder box
		add_static_seg(vec2(800, 800), vec2(800, 584))
		add_static_seg(vec2(800, 584), vec2(280, 560))
#		add_static_seg(vec2(60, 78), vec2(60, 100))
		# feeder chute
		add_static_seg(vec2(280, 560), vec2(240, 520), 0)
		add_static_seg(vec2(240, 520), vec2(240, 480), 0)
		add_static_seg(vec2(192, 800), vec2(192, 480), 0)
		# mechanical feeder bits
		add_spinner(vec2(560, 568), 4)
		add_flip_flop(vec2(216, 424))
		
		# right chute
		add_static_seg(vec2(216, 400), vec2(216, 112)) # left
		add_static_seg(vec2(320, 400), vec2(264, 160)) # right
		add_static_seg(vec2(216, 112), vec2(264, 96)) # bottom
		# bounce line
		body = CP::Body.new(Float::INFINITY, Float::INFINITY)
		body.v = vec2(0, 840)
		@space.add_shape(CP::Shape::Segment.new(body, vec2(280, 40), vec2(360, 40), 0))
		# catcher
		add_static_seg(vec2(480, 472), vec2(600, 456))
		add_static_seg(vec2(648, 456), vec2(648, 536))
		
		add_tipper(vec2(520, 360))
		
		# zigzag
		add_static_seg(vec2(800, 0), vec2(800, 400), 0) # wall right
		add_static_seg(vec2(400, 0), vec2(400, 240), 0) # wall left
		add_static_seg(vec2(480, 200), vec2(800, 240)).surface_v = vec2(-160, 0)
		add_static_seg(vec2(400, 120), vec2(720, 80)).surface_v = vec2(160, 0)
		add_static_seg(vec2(480, -40), vec2(800, 0)).surface_v = vec2(-160, 0)
		
		10.times do|i|
			y = 360 - 40*i
			add_flipper(vec2(176, y), true)
			add_flipper(vec2(80, y), false)
		end
	end
	
	def add_tipper(pos)
		tipper = Tipper.new(pos)
		tipper.add_to_world(self)
	end

	def add_static_seg(a, b, u=1)
		shape = CP::Shape::Segment.new(STATIC_BODY, a, b, 0)
		shape.u = u
		@space.add_static_shape(shape)
	end
	
	def add_flip_flop(pos)
		FlipFlop.new(pos).add_to_world(self)
	end
	
	def add_flipper(pos, right)
		Flipper.new(pos, right).add_to_world(self)
	end
	
	def add_spinner(pos, w)
		spinner = Spinner.new(pos, 4)
		spinner.add_to_world(self)
		@updatables << spinner
	end
	
	def add_ball(pos)
		ball = MenuBall.new(pos)
		ball.body.m = 1.0
		ball.body.i = CP.moment_for_circle(1, 24, 0, vec2(0,0))
		
		@space.add_object(ball)
		@balls << ball
		@balls_node.children << ball.view.view_node
	end
	
	def update()
		@ticks += 1
		# add balls to feeder
		add_ball(vec2(560 + rand*160, 720)) if @ticks%35 == 0
		
		dt = 1.0/120.0
		2.times do
			@updatables.send_each(:update, dt)
			@space.step(dt)
		end
		
		@balls.delete_if do|ball|
			if ball.body.p.y < -24
				@space.remove_object(ball)
				@balls_node.children.delete(ball.view.view_node)
				next true
			end
			
			next false
		end
	end
end


