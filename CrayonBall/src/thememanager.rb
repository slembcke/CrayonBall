module ThemeManager
	Themes = []

	def self.theme
		@theme
	end
	
	def self.theme=(theme)
		@theme = theme
		save
	end
	
	def self.border_textures
		@theme.border_textures
	end
	
	def self.ball_node_for_color(color_index)
		@theme.ball_loader.load[color_index]
	end
	
	def self.wildcard_ball_node
		@theme.ball_loader.load.last
	end
	
	def self.purchased?(theme)
		@theme_purchases[theme.name]
	end
	
	def self.afford?(theme)
		Stats[:currentCash] >= theme.coins || DEBUG_MODE
	end
	
	def self.buy(theme)
		return if purchased?(theme)
		Stats[:currentCash]-=theme.coins
		Stats.save
		@theme_purchases[theme.name] = true
		save
		self.theme=theme
	end
	
	ThemePath = File.join(GAME_STORAGE_PATH, 'themes.ser')
	
	def self.load
		@theme_purchases = Hash.new
		
		Themes.each{|theme|
			@theme_purchases[theme.name] = (theme.coins == 0)
		}

		# create the folder the first time.
		Dir.mkdir(GAME_STORAGE_PATH) unless File.exist?(GAME_STORAGE_PATH)
		# create the prefs file the first time
		unless File.exist?(ThemePath)
			@theme = Theme::Crayon
			save
		end
	
		open(ThemePath, 'r'){|f|
			purchases, current = Marshal.load(f)
			@theme_purchases.merge!(purchases)	
			
			@theme = Themes.find{|t| t.name == current }
			#puts "current theme is #{current} and that is theme: #{@theme}"
		}
		
		@theme = Theme::Crayon if @theme == nil
		
#		#puts "Loaded #{Themes.length} themes and purchase records #{@theme_purchases.length}"
#		@theme_purchases.each{|theme, val|
#			#puts "loaded #{theme} is #{val}"
#		}	
	end
	
	def self.save
		open(ThemePath, 'w'){|f| Marshal.dump([@theme_purchases, @theme.name], f)}
	end
	
end

module Theme

	attr_reader :coins
	
	def resource(file, &block)
		puts "loading resource " + self.name.to_s + " and it's a " + self.to_s
		AS::ResourceCache.load(File.join('gfx/themes', self.name, file), &block)
	end
	
	def border_textures
		[resource('bg.png'), resource('fg.png')]
	end
	
	def pretty_name
		"#{display_name} " + (ThemeManager.purchased?(self) ? "" : "(#{Misc.commas(coins)})")
	end
	
	def description
		""
	end
	
	def ball_loader
		# create an array of the base scene nodes shared by all balls
		# One for each texture in the ball texture map
		# 6 colors + the wildcard + the shading
		@ball_loader ||= AS::LazyLoader.new{
			ball_texture_map = resource('balls.png'){|key|
				Photon::Texture::MIP.new(key)
			}
			
			state_hash = {:texture => ball_texture_map, :blend => Photon::Blend::PremultAlpha}
			positions = [[0,1], [1,1], [2,1], [3,1], [0,0], [1,0], [2,0], [3,0]]
			
			# generate a vertex array with geometry for each ball in the texture
			var = Photon::VertexArray.new(2, 0, 2)
			positions.each{|col, row|
				var.texcoord((col + 0)*0.25, (row + 0)*0.5); var.vertex(-1, -1)
				var.texcoord((col + 1)*0.25, (row + 0)*0.5); var.vertex( 1, -1)
				var.texcoord((col + 1)*0.25, (row + 1)*0.5); var.vertex( 1,  1)
				var.texcoord((col + 0)*0.25, (row + 1)*0.5); var.vertex(-1,  1)
			}
			
			# generate the final array of nodes for drawing
			Array.new(positions.length){|i|
				Photon::Node::RenderState.construct(state_hash){
					Photon::Node::VAR.new(var, GL::QUADS, i*4, 4)
				}
			}
		}
	end
	
	def name
		str = super
		str.split("::").last
	end
	
	def self.extend_object(theme)
		super
		ThemeManager::Themes << theme
	end
		
	module Crayon extend Theme
		def self.display_name; "Crayon"; end		
		@coins = 0
	end
	
	module Classic extend Theme
		def self.display_name; "Classic"; end		
		def self.description; "Bringing  back the theme from the original ScribBall."; end
		@coins = 0
	end
	
	module ColorBlind extend Theme
		def self.display_name; "Color Blind"; end
		def self.description; "Color blind version of the pencil theme."; end
		@coins = 0
	end

	module EightBit extend Theme
		def self.display_name; "8-bit"; end
		@coins = 200
	end
			
	module DaVinci extend Theme
		def self.display_name; "The da Vinci Mode"; end		
		
		@coins = 400
	end
	
	module CoolDog extend Theme
		def self.display_name; "Cool Dog"; end
		def self.description; "Cool dog is my hero! (May have singlehandedly saved all of Milwaulkee)"; end
		@coins = 600
	end
	
	module Watercolor extend Theme
		def self.display_name; "Watercolor"; end		
		@coins = 1000
	end
	
	module Pencil extend Theme
		def self.display_name; "Pencil"; end		
		@coins = 1500
	end
	
	module Nedroid extend Theme
		def self.display_name; "Nedroid"; end
		def self.description; "Horse Party! Guest art by Anthony Clark. http://nedroid.com/"; end
		@coins = 2000
	end
		
	module Bright extend Theme
		def self.display_name; "Bright"; end		
		def self.description; "A simple and clean mode from the iPhone version."; end
		@coins = 3000
	end

	module Monet extend Theme
		def self.display_name; "Claude Monet"; end		
			@coins = 5000
	end

	module VanGogh extend Theme		
			def self.display_name; "Vincent van Gogh"; end		
			@coins = 10_000
	end

	module Eric extend Theme
		def self.display_name; "Eric Feurstein"; end		
		def self.description; "Belly of the beast. Guest art by Eric Feurstein. http://mrcolossal.livejournal.com/"; end
		@coins = 15_000
	end

	module Mondrian extend Theme
		def self.display_name; "Piet Mondrian"; end		
		
		@coins = 20_000
	end
		
end

ThemeManager.load
