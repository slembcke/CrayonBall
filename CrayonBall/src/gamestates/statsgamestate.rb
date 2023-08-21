class StatsStateDelegate
	
	def initialize(prevState)
		@font = AS::ResourceCache.load(:font, 'gfx/font')

		mc = AS::MenuController.new
		@menu_controller = mc
		mc.add_item("Back", 700, 40, 0.25, true){
			AS::GameState.stop(BaseState.new(prevState))
		}
		
		@actions = AS::Event::ActionListener.new(MenuStateDelegate.sharedActions)

		mc.add_actions(@actions)
		@background = MenuBackground.instance

		@scene = Photon::Node::Array.new([
			AS::QuickTextNode.new(@font, "Play Statistics", 400, 550, 0.45, Photon::Color::Black, true),
			mc,
		])
		
		@badge_coords = Hash.new
	
		Badge::BadgeClasses.each_with_index{|bClass, i|
			tex = AS::ResourceCache.load('gfx/badgesTiny/' + bClass.texture_name)
			
			x, y = i * 70+ 55, 50
			text = "#{Stats[bClass.name.to_sym]}"
			add_badge(tex, x, y)
			@scene.children << AS::QuickTextNode.new(@font, text, x, y - 35, 0.15, Photon::Color::Black, true)

			@badge_coords[vec2(x, y)] = bClass.display_name

		}
				

		spacing = 33
		originalTop = 480+spacing
		side = 20
		top = originalTop
		add_stat(side, top -= spacing, "Longest Single Game: #{time(:longestSingleGame)}")
		
		add_stat(side, top -= spacing, "Total time played: #{time(:totalTimePlayed)}")
		add_stat(side, top -= spacing, "Classic Time Played: #{time(:timePlayedClassic)}")
		add_stat(side, top -= spacing, "Overflow Time Played: #{time(:timePlayedOverflow)}")
		add_stat(side, top -= spacing, "Injection Time Played: #{time(:timePlayedDrop)}")
		add_stat(side, top -= spacing, "Shuffle Time Played: #{time(:timePlayedGrab)}")
		@watchingStats = "Time Admiring Stats: #{time(:timeWatchingStats)}"
		add_stat(side, top -= spacing, @watchingStats)
		
		add_stat(side, top -= spacing, "Total Games Played: #{Misc.commas(Stats[:totalGamesPlayed])}")
		add_stat(side, top -= spacing, "Classic Games Played: #{Misc.commas(Stats[:gamesPlayedClassic])}")
		add_stat(side, top -= spacing, "Overflow Games Played: #{Misc.commas(Stats[:gamesPlayedOverflow])}")
		add_stat(side, top -= spacing, "Injection Games Played: #{Misc.commas(Stats[:gamesPlayedDrop])}")
		add_stat(side, top -= spacing, "Shuffle Games Played: #{Misc.commas(Stats[:gamesPlayedGrab])}")

		add_stat(side, top -= spacing, "Total Coins Made: #{Misc.commas(Stats[:totalCashEver])}")
		
		top = originalTop
		side = 440
		add_stat(side, top -= spacing, "Current Coins: #{Misc.commas(Stats[:currentCash])}")
		add_stat(side, top -= spacing, "Biggest Combo Score: #{Misc.commas(Stats[:biggestCombo])}")
		add_stat(side, top -= spacing, "Longest Combo Chain: #{Misc.commas(Stats[:longestCombo])}")
		add_stat(side, top -= spacing, "Total Balls Popped: #{Misc.commas(Stats[:totalBallsPopped])}")
		add_stat(side, top -= spacing, "Balls Clicked: #{Misc.commas(Stats[:totalBallsClicked])}")

		add_stat(side, top -= spacing, "Bombs Exploded: #{Misc.commas(Stats[:bombsExploded])}")
		add_stat(side, top -= spacing, "Bombs Popped: #{Misc.commas(Stats[:bombsPopped])}")
		add_stat(side, top -= spacing, "Lightnings: #{Misc.commas(Stats[:stormLightning])}")
		add_stat(side, top -= spacing, "Windstorms: #{Misc.commas(Stats[:stormWind])}")

		add_stat(side, top -= spacing, "Balls Pushed: #{Misc.commas(Stats[:pushBalls])}")
		add_stat(side, top -= spacing, "Locks Removed: #{Misc.commas(Stats[:ballsUnlocked])}")
		add_stat(side, top -= spacing, "Wildcards Clicked: #{Misc.commas(Stats[:wildcardsClicked])}")


		
		@mouseover_text = ""
		@mouseover_matrix = Photon::Matrix.new #later, we will .translate!(x, y) this!
		
		scale = 0.18
		@mouseover_node = Photon::Node::Matrix.construct(GL::MODELVIEW, @mouseover_matrix){
			Photon::Node::Array.new([		
				AS::QuickTextNode.new(AS::ResourceCache.load(:font, 'gfx/font_outline'), @mouseover_text, 0, 40, scale*2.69, Photon::Color::White, true),
				AS::QuickTextNode.new(AS::ResourceCache.load(:font, 'gfx/font'), @mouseover_text, 0, 40, scale, Photon::Color::Black, true),
			])
		}
		
		@scene.children << @mouseover_node
	
		@actions.action(AS::Event.source_for_mouse){|event|
			@mouseover_matrix.reset!.translate!(*event.pos)
			
			mouse = vec2(*event.pos)
			@badge_coords.each_pair{|coord, name|
				if mouse.near?(coord, 17)
					@mouseover_text.replace(name)
					break
				end
				@mouseover_text.replace("")
			}
			
		}
	end
	
	def time(symbol)
		as_time(Stats[symbol])
	end
	
	def as_time(time)
		seconds = (time / 60) % 60
		min = (time / 3600) % 60
		hours = (time / (3600*60)) % 24
		days = (time / (3600*60*24))

		dayString = (days > 1 ? "#{days} days, " : (days > 0 ? "#{days} day, " : ""))
		
		#hourString = (hours > 1 ? "#{hours} hrs, " : (hours > 0 ? "#{hours} hr, " : ""))
		#minString = (min > 1 ? "#{min} mins, and " : (min > 0 ? "#{min} min, and " : ""))
		#secString = (seconds > 1 ? "#{seconds} secs." : (seconds > 0 ? "#{seconds} sec." : "None"))
		
		hourString = (hours > 0 ? "#{hours}:" : "")
		minuteFormat = (hours > 0 ? "%02d" : "%d")
		rest = sprintf(minuteFormat + ":%02d", min, seconds)
		
		return dayString + hourString + rest
	end
	
	def add_stat(x, y, text)
		@scene.children << AS::QuickTextNode.new(@font, text, x, y, 0.15, Photon::Color::Black, false)
	end
	
	def add_badge(tex, x, y)
	
		matrix = Photon::Matrix.new.translate!(x, y)
		
		@scene.children << 
			Photon::Node::Matrix.construct(GL::MODELVIEW, matrix){
				Photon::Node::RenderState.construct({:blend => Photon::Blend::PremultAlpha}) {
					Photon::Node::SpriteTemplate.new(tex, tex.w/2, tex.h/2)
				}
			}
	end
	
	
	def update
		@background.update
		@menu_controller.update
		
		Stats[:timeWatchingStats]+=1
		@watchingStats.replace("Time Admiring Stats: #{time(:timeWatchingStats)}")
		
		
	end
	
	def exit_state
		Stats.save
	end
		
	def input(event)
		@actions.call(event)
	end
	
	def draw
		AS.clear_screen
		@background.view_root.render    
		@scene.render
	end
end
