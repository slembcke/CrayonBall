class HighScore
	attr_accessor :name, :points, :date
	attr_reader :badges
	
	def initialize(name, points, badges, date)
		@name, @points, @date = name, points, date

		@badges = badges.find_all{|b| b.count > 0}
		
		raise "Badges are not badges, there's a " + @badges.first if @badges.first.class >= Badge
		raise "points are not points, it is a " + points.class.to_s if points.class >= Integer
	end

	def <=>(other)
		points <=> other.points
	end
	
	def to_s
		"#{name} - #{Misc.commas(points)} points"
	end
	
	def score_string
		"#{Misc.commas(points)} points"
	end

end


class HighScoreList
	attr_reader :scores
	Modes = [
		:classic,
		:overflow, 
		:drop,
		:grab,
	]
	HighScorePath = {
		:classic => File.join(GAME_STORAGE_PATH, 'scoresClassic.ser'),
		:overflow =>  File.join(GAME_STORAGE_PATH, 'scoresOverflow.ser'),
		:drop =>  File.join(GAME_STORAGE_PATH, 'scoresDrop.ser'),
		:grab =>  File.join(GAME_STORAGE_PATH, 'scoresGrab.ser'),
	}

	def initialize(max_scores, mode)
		@max_scores = max_scores
		@mode = mode
		@scores = []

		raise "Invalid mode <#{mode}>!" if !Modes.include?(mode)

		# create the folder the first time.
		Dir.mkdir(GAME_STORAGE_PATH) unless File.exist?(GAME_STORAGE_PATH)
		# create the prefs file the first time
		save unless File.exist?(HighScorePath[mode])
	
		open(HighScorePath[@mode], 'r'){|f|
			@scores = (Marshal.load(f))			
		}
	end
	
	def save
		open(HighScorePath[@mode], 'w'){|f| Marshal.dump(@scores, f)}
	end
	
	def inspect
		@scores.inspect
	end
	
	def add(name, score)
		points = score.score
		highScore = HighScore.new(name, points, score.badges, Time.now)
		@scores = (@scores + [highScore]).sort.reverse[0...@max_scores]
		
		return highScore
	end
	
	def highscore?(score)
		points = score.score
		(@scores.length < @max_scores) or (points > @scores.last.points)
	end
	
	def clear
		File.delete(HighScorePath[@mode]) if File.exist?(HighScorePath[@mode])

		@scores = []
	end
	
end

HighScores = {
	:classic => HighScoreList.new(7, :classic),
	:overflow => HighScoreList.new(7, :overflow),
	:drop => HighScoreList.new(7, :drop),
	:grab => HighScoreList.new(7, :grab),
}
