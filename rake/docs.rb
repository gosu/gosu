# For some reason, YARD::Rake::YardocTask does not work for me at all.
desc "Build Ruby reference with YARD"
task :ruby_docs do
  sh "yardoc"
end

desc "Build C++ reference with doxygen"
task :cpp_docs do
  sh "cd reference && doxygen &> /dev/null"
end
