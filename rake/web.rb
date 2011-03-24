if ENV['USER'] == 'jlnr' and `uname`.chomp == 'Darwin' then
  CGI_PATH = '/Library/WebServer/CGI-Executables/libgosu.org'
  
  namespace :web do
    task :forum_maintenance do
      sh "cd #{CGI_PATH}/mwf && perl cron_jobs.pl"
      sh "cd #{CGI_PATH}/mwf && perl cron_subscriptions.pl"
      sh "cd #{CGI_PATH}/mwf && perl cron_rss.pl"
    end
    
    task :rebuild_index do
      sh "cd website && ruby potd.rb && erb -r potd_values.rb index.html.erb > index.html"
    end
    
    task :docs => [:ruby_docs, :cpp_docs] do
      rm_rf "website/rdoc"
      rm_rf "website/cpp"
      sh "mv reference/rdoc reference/cpp website"
    end
    
    task :update => [:forum_maintenance, :docs, :rebuild_index]
  end
end