
module AS
	
	class QuickTextNode < Photon::Node::Matrix
		
		attr_reader :textNode
		attr_reader :str, :x, :y
		attr_reader :scale
		attr_reader :font
		
		def initialize(font, str, x, y, scale=1, color=Photon::Color::Red, centered = false)
			@font, @str, @original_x, @original_y, @scale, @color = font, str, x, y, scale, color
			@centered = centered
			
			@color = Photon::Color.new(*@color.to_rgba())
			@matrix = Photon::Matrix.new

			self.str = str;

			@textNode = Photon::Node::Text.new(font, str) 
			render = Photon::Node::RenderState.construct({:blend => Photon::Blend::PremultAlpha, :color => @color}){
				@textNode
			}
			
			super(GL::MODELVIEW, matrix, render)
		end
		
		def x=(x)
			@x = x
			update_matrix
		end

		def y=(y)
			@y = y
			update_matrix
		end
		
		def color=(color)
			@color.color = color
		end
		
		def str=(str)
			@str.replace(str)
			recenter
		end
		
	private 
		def recenter
			center_x, center_y = get_string_center
			@x = @original_x - (center_x * scale)
			@y = @original_y - (center_y * scale)
			
			update_matrix()
		end

		def get_string_center
			return @font.string_center(@str) if @centered
			return 0, 0
		end
	
		def update_matrix()
			@matrix.reset!.sprite!(@x, @y, @scale, @scale, 0)	
		end
		
	end
	
	class MenuTextNode < Photon::Node::Array
		attr_accessor :active, :controller
		
		def initialize(string, x, y, scale, block, centered = false)
			@blur_color = Photon::Color::White
			@focus_color = Photon::Color.new(69.0/255.0, 178.0/255.0, 223.0/255.0)
			@select_color = Photon::Color.new(231.0/255.0, 144.0/255.0, 59.0/255.0)

			font = AS::ResourceCache.load(:font, 'gfx/font')
			text = QuickTextNode.new(font, string, x, y, scale, Photon::Color::Black, centered)
			
			outline_font = AS::ResourceCache.load(:font, 'gfx/font_outline')
			@outline_node = QuickTextNode.new(outline_font, string, x, y, scale*2.69, @blur_color, centered)
			self.str = string
			
			@active = false
			@block = block
			
			@mouse_in = false
			
			super([@outline_node, text])
		end
		
		def method_missing(method, *args)
			@outline_node.send(method, *args)
		end
	
		def str=(str)
			super(str)
			@bounds = font.string_bbox(str)
		end
	
		def check_bbox(mx, my)
			l, r, b, t = *(@bounds)
			mx = (mx - x)/scale 
			my = (my - y)/scale 

			#puts "vars: #{mx}, #{my}, #{l}, #{r}, #{b}, #{t}."
			return l < mx && mx < r && b < my && my < t
		end
		
		def set_inside_callback(&block)
			@inside_callback = block
		end
		
		def rollover(mx, my)
			inside = check_bbox(mx, my)
			
			# do some effects
			if !@mouse_in and inside and self.enabled
				AS::ResourceCache.load('sfx/pop.ogg').play(2.0**((rand(5) - 2)/8.0))
				
				l, r, b, t = *(@bounds)
				center = vec2(l + r, b + t)/2 + vec2(x, y)
				@controller.add_effect(MenuParticleEffect.new(center, @focus_color)) if @controller
				
				@inside_callback.call if @inside_callback
			end
			
			@mouse_in = inside
			self.color=(inside ? @focus_color : get_blur_color)
		end

		def get_blur_color
			@active ? @select_color : @blur_color 
		end

		def activate
			return if !self.enabled
			
			AS::ResourceCache.load('sfx/ploop.ogg').play
			@block.call
			@active = true
		end
		
		def deactivate
			@active = false
			self.color=@blur_color
		end
		
		def start_active
			@active = true
			self.color=@select_color		
		end
				
	end 
	
	class MenuController < Photon::Node::Array
	
		attr_reader :view_root
		
		def initialize
			@font = AS::ResourceCache.load(:font, 'gfx/font')

			@effects = []
			@items = []
			
			super([
				Photon::Node::Map.new(@effects){|effect| effect.view_node},
				Photon::Node::Array.new(@items),
			])
		end
		
		def add_item(string, x, y, scale, centered = false, &block)
			item = MenuTextNode.new(string, x, y, scale, block, centered)
			
			item.controller = self
			@items << item
			
			return item
		end
		
		def make_active(i)
			@items[i].start_active
		end
		
		def get_active_item()
			@items.find(){|i| i.active }		
		end
		
		def rollover(mx, my)
			@items.each{|i|
				i.rollover(mx, my)
			}
		end
		
		def click(mx, my)
			foundOne = false
			@items.each{|i|
				if i.check_bbox(mx, my)
					foundOne = true
					i.activate
				end
			}
			if foundOne
				@items.each{|i|
					i.deactivate if !i.check_bbox(mx, my)
				}
			end
		end
		
		def add_actions(list)
			list.action(AS::Event::MouseButtonEvent.source_id(1)) do |event|
				click(*event.pos) if event.state
			end
	
			list.action(AS::Event::MouseMotionEvent.source_id()) do |event|
				rollover(*event.pos)
			end
		end
		
		def add_effect(effect)
			@effects << effect
		end
		
		def update
			@effects.send_each(:tick)
			@effects.reject!{|effect| effect.done?}
			@effects.send_each(:update)
		end
	
	end

end