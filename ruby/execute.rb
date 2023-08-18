AL.Listenerfv(AL::POSITION, [0,0,0])
AL.Listenerfv(AL::VELOCITY, [0,0,0])
AL.Listenerfv(AL::ORIENTATION, [0,1,0,0,0,1])

AS::GameState.event_source = AS::Event

# start the main loop of the game code. Here we handle any exceptions
# that would crash the game code, and we shut down aerosol gracefully.
begin
	# check if the first argument is an option
	# if not, assume it's a starter script
	if ARGV[0] and ARGV[0][0] != ?-
		require(ARGV.shift)
	else
		require('src/main.rb')
	end
rescue SystemExit, Interrupt
  Kernel.puts "Exit request caught."
rescue Exception
  #	puts "Unhandled exeption caught."
  Kernel.puts $!.message
  Kernel.puts $!.backtrace.collect{|frame| "\t" + frame.sub(AS::LIB_DIR, "$(AS::LIB_DIR)")}
end

Kernel.print "Shutting down Aerosol ... "
AS.exit
Kernel.puts "Successful!"

GC.start
