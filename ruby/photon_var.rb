module Photon
	class VertexArray
		attr_reader :num_vertex, :num_color, :num_texcoord
		
		def initialize(num_vertex, num_color=0, num_texcoord=0)
			raise(ArgumentError, "num_vertex must be 2, 3 or 4 was #{num_vertex}") unless [2,3,4].include?(num_vertex)
			raise(ArgumentError, "num_color must be 3 or 4 was #{num_color}") unless [0,3,4].include?(num_color)
			raise(ArgumentError, "num_texcoord must be 2, 3 or 4 was #{num_texcoord}") unless [0,2,3,4].include?(num_texcoord)
			
			@num_vertex, @num_color, @num_texcoord = num_vertex, num_color, num_texcoord
			
			@texcoord = [0, 0, 0]
			@color = [0, 0, 0, 0]
			@VAR = []
			
			@last_chunk = 0
			
			@packed = nil
		end
		
		def texcoord(x, y, z=0)
			@texcoord = [x, y, z]
			
			return self
		end
		
		def color(r, g, b, a=1)
			@color = [r, g, b, a]
			
			return self
		end
		
		def vertex(x, y, z=0, w=1)
			@VAR << ([x, y, z, w][0, @num_vertex] + @color[0, @num_color] + @texcoord[0, @num_texcoord])
			
			return self
		end
		
		def var
			@VAR
		end
		
		def length
			@VAR.length
		end
		
		def pack
			if not frozen?
				@packed = @VAR.flatten.pack("f*")

				self.freeze
				@VAR.freeze
			end
			
			return @packed
		end
		
		def make_node(mode)
			Photon::Node::VAR.new(self, mode, 0, @VAR.length)
		end
		
		def chunk
			count = @VAR.length - @last_chunk
			chunk = Chunk.new(self, @last_chunk, count)
			
			@last_chunk = @VAR.length
			return chunk
		end
		
		class Chunk
			def initialize(var, first, count)
				@var, @first, @count = var, first, count
			end
			
			def make_node(mode)
				Photon::Node::VAR.new(@var, @first, @count)
			end
		end

		def self.for_copy_rect(sx, sy, sw, sh, dx, dy, dw, dh)
			sright = sx + sw
			stop   = sy + sh
	
			dright = dx + dw
			dtop   = dy + dh
	
			var = self.new(2, 0, 2)
			var.texcoord(    sx,   sy).vertex(    dx,   dy)
			var.texcoord(sright,   sy).vertex(dright,   dy)
			var.texcoord(sright, stop).vertex(dright, dtop)
			var.texcoord(    sx, stop).vertex(    dx, dtop)
			
			return var
		end
	
		def self.for_sprite(tex, kx, ky)    
			w, h = [tex.w, tex.h]
			tw, th = (tex.tex_target == GL::TEXTURE_RECTANGLE_EXT) ? [w, h] : [1, 1]
	
			return for_copy_rect(0, 0, tw, th, -kx, -ky, w, h)
		end
	end
end
