module LoadScreen

	def self.show
	
		tex = AS::ResourceCache.load('gfx/hmsLogo.png')
		dx = (800 - tex.w) / 2
		dy = (600 - tex.h)	/ 2
		
		logo = Photon::Node::RenderState.construct(:color => Photon::Color::White, :texture => tex, :blend => Photon::Blend::PremultAlpha) do
			Photon::Node::CopyRect.new_for_texture(tex, dx, dy)
		end
		
		l, b, w, h = *BaseState.screen_bounds #[-80.0, -0.0, 960.0, 600]
		#puts BaseState.screen_bounds.inspect
		tex = AS::ResourceCache.load('gfx/chipmunkLogo.png')
		dx = w - tex.w - 20 + l
		dy = 20 
		
		chimpmonk = Photon::Node::RenderState.construct(:color => Photon::Color::White, :texture => tex, :blend => Photon::Blend::PremultAlpha) do
			Photon::Node::CopyRect.new_for_texture(tex, dx, dy)
		end
		
		AS.clear_screen
		logo.render
		chimpmonk.render
		AS.swap_buffers
	end
	
end