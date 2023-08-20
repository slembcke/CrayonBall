class StatClass

		ALL_STATS = {
			:biggestCombo => 0,
			:longestCombo => 0,
			:totalBallsPopped => 0,
			:totalBallsClicked => 0,
			:totalTimePlayed => 0,
			:timePlayedClassic => 0,
			:timePlayedOverflow => 0,
			:timePlayedDrop => 0,
			:timePlayedGrab => 0,
			:timeWatchingStats => 0,
			:totalGamesPlayed => 0,
			:gamesPlayedClassic => 0,
			:gamesPlayedOverflow => 0,
			:gamesPlayedDrop => 0,
			:gamesPlayedGrab => 0,
			:longestSingleGame => 0,
			:bombsExploded => 0,
			:bombsPopped => 0,
			:stormLightning => 0,
			:stormWind => 0,
			:pushBalls => 0,
			:ballsUnlocked => 0,
			:wildcardsClicked => 0,
			:totalCashEver => 0,
			:currentCash => 0,
			
			:ComebackBadge => 0,
			:TriggerFingerBadge => 0,
			:NoSpecialsBadge => 0,
			:EmptyBadge => 0,
			:GigaComboBadge => 0,
			:UltraComboBadge => 0,
			:MegaComboBadge => 0,
			:SurvivalBadge => 0,
			:ScoreAchievementBadge => 0,
		}

	def initialize
		@stats = ALL_STATS.clone
		
		load
		
		if self[:ScribBallVersion] != VersionString
			# reset the play time if the version number changes on a
			# unregistered version. Allow the trial period to reset.
			if !$registered
				puts "resetting play time for update"
				self[:totalTimePlayed] = 0
			end
			
			self[:ScribBallVersion] = VersionString
			save
		end
	end
	
	def [](symbol)
		@stats[symbol]
	end

	def []=(symbol, value)
		@stats[symbol] = value
	end

	StatsPath = File.join(GAME_STORAGE_PATH, 'stats.ser')
	
	def load
		# create the folder the first time.
		Dir.mkdir(GAME_STORAGE_PATH) unless File.exist?(GAME_STORAGE_PATH)
		# create the prefs file the first time
		save unless File.exist?(StatsPath)
	
		open(StatsPath, 'r'){|f|
			@stats.merge!(Marshal.load(f))			
		}
	end
	
	def save
		open(StatsPath, 'w'){|f| Marshal.dump(@stats, f)}
	end
	
	def inspect
		@stats.inspect
	end
	
end

Stats = StatClass.new

