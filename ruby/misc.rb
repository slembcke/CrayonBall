module Misc
	def self.commas(n)
		str = n.to_s
		((str.length - 1)/3).times{|i| str.insert(-i*4 - 4, ',')}
		
		str
	end
	
	def self.web_page(site="http://howlingmoonsoftware.com/crayonball.php")
		if GamePrefs.fullscreen?
			GamePrefs.screen_value = 0 # set it to small windowed
		end
		puts 'res change'
		
		case RUBY_PLATFORM
		when /darwin/: system("open " + site)
		when /mingw/: Thread.new{system("rundll32.exe url.dll,FileProtocolHandler " + site)}
		end
	end
	
	def self.remaining_trial
		minutes = 30 - Stats[:totalTimePlayed]/(60*60)
	end
end
