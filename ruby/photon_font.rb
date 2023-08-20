class Photon::Font
	# gx/gy glyph offsets, tx/ty texture offsets
	Metrics = Struct.new(:gx, :gy, :tx, :ty, :w, :h, :advance)
	
	attr_reader :tex, :line_skip, :ascent, :descent
	attr_reader :VAR
	attr_reader :packed_indexes
	attr_reader :packed_advances
	attr_reader :line_height
	
	def self.open(filename)
		tex = Photon::Texture::Rect.new(filename + '.png')
		new(tex, filename + '.metrics')
	end
	
	def line2ints(line)
		line.split.map{|str| str.to_i}
	end

	def initialize(tex, metrics_filename)
		case tex
		when Photon::Texture::Rect: sx, sy = 1, 1
		when Photon::Texture: sx, sy = tex.w, tex.h
		else raise 'tex must be an AS::Texture'
		end
		
		@tex = tex
		load_glyphs(metrics_filename, 1.0/sx, 1.0/sy)
		@line_height = @ascent - @descent
	end
	
	def load_glyphs(filename, sx, sy)
		indexes = {}
		metrics = {}

		File.open(filename, 'r') do|metrics_file|
			@pad, @line_skip, @ascent, @descent = *line2ints(metrics_file.readline)
			
			@VAR = Photon::VertexArray.new(2, 0, 2)
			for line in metrics_file do
				#split out a metric line to it's parts
				index, *other_metrics = *line2ints(line)
				metrics[index] = Metrics.new(*other_metrics)
				gx, gy, tx, ty, w, h, advance = *other_metrics
				
				#add the glyph to the VAR
				indexes[index] = @VAR.length
				@VAR.texcoord((tx   -   @pad)*sx, (ty   -   @pad)*sy)
				@VAR.vertex(   gx   -   @pad    ,  gy   -   @pad    )

				@VAR.texcoord((tx + w + @pad)*sx, (ty   -   @pad)*sy)
				@VAR.vertex(   gx + w + @pad    ,  gy   -   @pad    )

				@VAR.texcoord((tx   -   @pad)*sx, (ty + h + @pad)*sy)
				@VAR.vertex(   gx   -   @pad    ,  gy + h + @pad    )

				@VAR.texcoord((tx + w + @pad)*sx, (ty + h + @pad)*sy)
				@VAR.vertex(   gx + w + @pad    ,  gy + h + @pad    )
			end
		end
		
		#build final arrays
		metrics.default = metrics[?~]		
		@metrics = Array.new(256){|i| metrics[i]}

		indexes.default = indexes[?~]
		@indexes = Array.new(256){|i| indexes[i]}

		@packed_indexes = @indexes.pack('I*')
		@packed_advances = Array.new(256){|i| metrics[i].advance}.pack('f*')
	end
	
	def string_bbox(str)#; breakpoint
		return [0,0,0,0] if str.empty?
		
		lines = str.split("\n")
		
		#find leftmost glyph start
		left = lines.map{|line| line.empty? ? 0 : @metrics[line[0]].gx}.min
		
		#find rightmost glyph end
		right = lines.map {|line|
			last_index = line.length - 1 #grab the advance from all but the last char
			tmp_right = (0...last_index).inject(0){|sum, i| sum + @metrics[line[i]].advance}
			
			#grab the rightmost edge from the last glyph
			glyph = @metrics[line[last_index]]
			tmp_right + glyph.w + glyph.gx
		}.max
		
		[left, right, @descent - @line_skip*(lines.length - 1), @ascent]
	end
	
	def string_width(str)
		l, r, b, t = *string_bbox(str)
		return l + r
	end
	
	def string_center(str)
		l, r, b, t = *string_bbox(str)
		return [(l + r)/2.0, (b + t)/2.0]
	end
	
	module AS::ResourceCache::FontLoader
		AS::ResourceCache::Loaders << self
		
		def self.match?(key)
			key.is_a?(Array) and key[0] == :font
		end
		
		def self.loader
			lambda{|key|
				sym, path = *key
				
				puts "loading font #{path}"
				tex = AS::ResourceCache.load(path + ".png")
				
				return Photon::Font.new(tex, path + ".metrics")
			}
		end
	end
end

#AS::Font.autoload(:DefaultFont, "base/font/loadDefaultFont.rb")