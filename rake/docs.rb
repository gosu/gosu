# For some reason, YARD::Rake::YardocTask does not work for me at all.
desc "Build Ruby reference with YARD"
task :ruby_docs do
  # Also work around the fact that yardoc does not work at all for me in 1.8
  # (Blaming a missing rdoc gem)
  # Oh, and not in 1.9.2 either...
  sh "rvm 1.9.3 do yardoc || yardoc"
end

desc "Build C++ reference with doxygen"
task :cpp_docs do
  sh "cd reference && doxygen &> /dev/null"
end
