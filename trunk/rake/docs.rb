require 'rake/rdoctask'

desc "Clean rdoc directory so all HTML will definitely be refreshed"
task :clean_rdoc do
  sh "rm -rf reference/rdoc/*"
end

Rake::RDocTask.new do |rd|  
  rd.main = "README.txt"
  rd.rdoc_dir = "reference/rdoc"
  rd.rdoc_files.include("reference/gosu.rb", "README.txt", "COPYING.txt", *Dir["reference/*.rdoc"])
  rd.title = "RubyGosu rdoc Reference"
  rd.template = "reference/rdoc-template/lib/allison"
end

desc "Build Ruby reference with rdoc"
task :ruby_docs => [:clean_rdoc, :rdoc] do
  # Make sure that there are no blatant syntax errors in gosu.rb
  sh "ruby reference/gosu.rb"
  # Copy the stylesheet over, wee!
  sh "cp reference/rdoc-style.css reference/rdoc/rdoc-style.css"
end

desc "Build C++ reference with doxygen"
task :cpp_docs do
  sh "cd reference && doxygen"
end

desc "Upload documentation to www.libgosu.org"
task :upload_docs => [:cpp_docs, :ruby_docs] do
  # Expect some errors here, for example copying the .svn subdirectories might break.
  sh "scp -C -r reference/cpp vu2040@eta.railshoster.de:~/htdocs/ || true"
  sh "scp -C -r reference/rdoc vu2040@eta.railshoster.de:~/htdocs/ || true"
end
