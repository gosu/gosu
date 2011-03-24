require 'rake/rdoctask'

desc "Clean rdoc directory so all HTML will definitely be refreshed"
task :clean_rdoc do
  sh "rm -rf reference/rdoc/*"
end

Rake::RDocTask.new do |rd|  
  rd.main = "README.txt"
  rd.rdoc_dir = "reference/rdoc"
  rd.rdoc_files.include("reference/gosu.rb", "README.txt", "COPYING", *Dir["reference/*.rdoc"])
  rd.title = "RubyGosu rdoc Reference"
  rd.template = "reference/rdoc-template/lib/allison"
end

desc "Build Ruby reference with rdoc"
task :ruby_docs => [:clean_rdoc, :rdoc] do
  # Copy the stylesheet over, wee!
  sh "cp reference/rdoc-style.css reference/rdoc/rdoc-style.css"
end

desc "Build C++ reference with doxygen"
task :cpp_docs do
  sh "cd reference && doxygen > /dev/null"
end
