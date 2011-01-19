#!/usr/bin/env ruby

# Usage: condor_run.rb <runs> <script>
#
#  * <runs>:  	number of runs  
#  * <script>:	script to be run
#
# Author: patmac@cs.utexas.edu (Patrick MacAlpine)

# Parse command line arguments 

if (ARGV.size < 2)
  STDERR.print "Usage: #{$0} <runs> <script>\n"
  exit(1)
end

# number of runs
$numRuns = ARGV[0].to_i 
$script = ARGV[1]

# ---------------------------------------

def run_on_condor(run, script) 
  condorContents = <<END_OF_CONDORFILE;

Executable = #{$script}
Universe = vanilla
#Environment = PATH=/lusr/bin:/bin:/usr/bin;
Requirements = Lucid

+Group = "GRAD"
+Project = "AI_OPENNERO"
+ProjectDescription = "opennero testing"

Input = /dev/null

END_OF_CONDORFILE
  
  condorContents = condorContents + "\n"
  
  condorContents = condorContents +
    "Error = error_#{run}.txt\n" +
    "Output = output_#{run}.txt\n" +
    "Log = log_#{run}.txt\n"
  
  condorContents = condorContents + "arguments = #{run}\n" +
    "Queue 1";

  #submit the job:
  print "Submitting job for #{script}_#{run}\n"
  print condorContents
  condorSubmitPipe = open("|condor_submit", 'w');
  condorSubmitPipe.write(condorContents)
  condorSubmitPipe.close
end

(0..$numRuns-1).each do |run|
  run_on_condor(run, $script)
end
