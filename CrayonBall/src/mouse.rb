class Mouse
	attr_reader :view_node
	
	CURSOR_NODE_LOADER = AS::LazyLoader.new{
		tex = AS::ResourceCache.load('gfx/cursor.png')
		
		Photon::Node::RenderState.construct(:blend => Photon::Blend::PremultAlpha){
			Photon::Node::SpriteTemplate.new(tex, 0, tex.h)
		}
	}
	
	SCALE = 0.25
	
	def initialize
		@matrix = Photon::Matrix.new.sprite!(400, 300, SCALE, SCALE, 0)
		@view_node = Photon::Node::Matrix.new(GL::MODELVIEW, @matrix, CURSOR_NODE_LOADER.load)
	end
	
	def update_pos(x, y)
		@matrix.reset!.sprite!(x, y, SCALE, SCALE, 0)
	end
end