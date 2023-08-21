#require 'enumerator'

class ThemePickerStateDelegate
	
	class ThemePicker
		def self.build_pickers
			# removing dependence on the Enumerator module
#			Enumerable::Enumerator.new(ThemeManager::Themes, :each_slice, 4).map{|chunk| ThemePicker.new(chunk)}
			
			ary = ThemeManager::Themes
			count = ary.length
			list = Array.new((count + 3)/4){[]}
			
			ary.each_with_index do|elt, i|
				list[i/4] << elt
			end
			
			p list
			return list.map{|chunk| ThemePicker.new(chunk)}
		end
		
		attr_reader :view_node
		
		BORDER_NODES_LOADER = AS::LazyLoader.new{
			tex = AS::ResourceCache.load('gfx/theme_picker.png')
			w, h = tex.w, tex.h/4
			
			Array.new(4){|i| Photon::Node::CopyRect.new(tex, 0, i*h, w, h, 0, 0, w, h)}
		}
		
		def border_node_for_theme(theme)
			nodes = BORDER_NODES_LOADER.load

			if ThemeManager.theme == theme
				nodes[2]
			elsif ThemeManager.purchased?(theme)
				nodes[3]
			else
				if ThemeManager.afford?(theme)
					nodes[1]
				else
					nodes[0]
				end
			end
		end
		
		BORDER_X = 200
		BORDER_Y = 380
		ROW_OFFSET = 80
		
		def initialize(themes)
			@themes = themes
			@descriptions = themes.map{|theme| theme.description}
			@rollover_colors = Array.new(4){Photon::Color::Black.clone}
			
			font = AS::ResourceCache.load(:font, 'gfx/font')

			label_matrix = Photon::Matrix.new.sprite!(90, 30, 0.17, 0.17, 0)
			
			nodes = Array.new(themes.length){|i|
				theme = themes[i]
				label = theme.pretty_name
				
				matrix = Photon::Matrix.new.translate!(BORDER_X, BORDER_Y - ROW_OFFSET*i)
				Photon::Node::Matrix.construct(GL::MODELVIEW, matrix){
					Photon::Node::RenderState.construct(:blend => Photon::Blend::PremultAlpha){
						Photon::Node::Array.new([
							border_node_for_theme(theme),
							Photon::Node::Matrix.construct(GL::MODELVIEW, label_matrix){
								Photon::Node::RenderState.construct(:color => @rollover_colors[i]){
									Photon::Node::Text.new(font, label)
								}
							},
						])
					}
				}
			}
			
			@view_node = Photon::Node::Array.new(nodes)
		end
		
		def mouse_index(x, y)
			return unless 200 < x and x < 600
			
			i = -(y.to_i - BORDER_Y)/ROW_OFFSET + 1
			return i if 0 <= i and i < 4
		end
		
		def mouse_over(description, x, y)
			index = mouse_index(x, y)
			@rollover_colors.each_with_index{|color, i|
				color.color = (i==index ? Photon::Color::Red : Photon::Color::Black)
			}
			
			description.str = @descriptions[index] if index and index < @descriptions.length
		end
		
		def clicked_theme(x, y)
			i = mouse_index(x, y)
			@themes[i] if i
		end
	end
	
	def initialize()
		font = AS::ResourceCache.load(:font, 'gfx/font')
		
		title = AS::QuickTextNode.new(font, "Pick Your Theme!", 400, 530, 0.5, Photon::Color::Black, true)
		
		mc = AS::MenuController.new
		@menu_controller = mc
		mc.add_item("Done", 700, 40, 0.25, true){
			AS::GameState.stop(BaseState.new(MenuStateDelegate.get_state))
		}
		
		@coin_str = ""
		@description_node = AS::QuickTextNode.new(font, "", 400, 110, 0.15, Photon::Color::Black, true)
		@actions = AS::Event::ActionListener.new(MenuStateDelegate.sharedActions)

		mc.add_actions(@actions)
		@background = MenuBackground.instance
		
		reset
		
		@actions.action(AS::Event.source_for_mbutton(1), &method(:click))
		@actions.action(AS::Event.source_for_mouse){|event| current_picker.mouse_over(@description_node, *event.pos)}
		@prev = mc.add_item("prev", 650, 420, 0.25, true){set_picker_index(@picker_index - 1)}
		@next = mc.add_item("next", 650, 180, 0.25, true){set_picker_index(@picker_index + 1)}
		set_picker_index(0)
		
		@scene = Photon::Node::Array.new([
			title,
			mc,
			AS::QuickTextNode.new(font, @coin_str, 50, 50, 0.3, Photon::Color::Black),
			@description_node,
		])
	end
	
	def reset
		@theme_pickers = ThemePicker.build_pickers
		@coin_str.replace("Coins: " + Stats[:currentCash].to_s)
	end
	
	def set_picker_index(i)
		@picker_index = i
		
		@prev.enabled = (i != 0)
		@next.enabled = (i != @theme_pickers.length - 1)
	end
	
	def current_picker
		@theme_pickers[@picker_index]
	end
	
	def click(event)
		return if !event.state
		
		theme = current_picker.clicked_theme(*event.pos)
		if theme
			if ThemeManager.purchased?(theme)
				ThemeManager.theme = theme
				reset
				AS::ResourceCache.load('sfx/stomp.ogg').play
			elsif ThemeManager.afford?(theme)
				ThemeManager.buy(theme)
				reset
				AS::ResourceCache.load('sfx/stomp.ogg').play
			else
				AS::ResourceCache.load('sfx/clank.ogg').play
			end
		end
	end
	
	def update
		@background.update
		@menu_controller.update
	end
		
	def input(event)
		@actions.call(event)
	end
	
	def draw
		AS.clear_screen
		@background.view_root.render    
		@scene.render
		current_picker.view_node.render
	end
end
