if ENV['USER'] == 'jlnr' then
  desc "Publish OmniFocus tasks in ToDo Wiki page"
  task :todo do
    File.open "wiki/ToDo.wiki", "w" do |wiki_page|
      doc = Appscript.app("OmniFocus").default_document
      my_projects = doc.folders["Einzelprojekte"].projects.get
      gosu_projects = my_projects.select { |p| p.name.get =~ /^Gosu/ }
    
      wiki_page.puts "#summary List of upcoming To Do items, refreshed daily."
      wiki_page.puts "#labels Featured"
      wiki_page.puts
      wiki_page.puts "= To Do list ="
      wiki_page.puts
      wiki_page.puts "This list of tasks is exported automatically from my OmniFocus projects."
      wiki_page.puts "(Last update: #{Date.today})"
      wiki_page.puts
    
      $shown_tasks = []
    
      def wiki_page.list_tasks_in root, indent = 2
        root.tasks.get.each do |task|
          next if $shown_tasks.include? task or task.completed.get
          $shown_tasks << task
          puts "#{' ' * indent}* #{task.name.get}"
          list_tasks_in task, indent + 2 unless task.tasks.get.empty?
        end
      end  

      gosu_projects.each do |project|
        wiki_page.puts "== #{project.name.get} =="
        wiki_page.list_tasks_in project
      end
    end
  
    `svn update wiki` # Just for cleanliness' sake.
  
    diff = `svn diff wiki`
    if diff.count("\n") == 13 and diff =~ /Last update/ then
      # We have only changed the date, just revert.
      sh "svn revert wiki/ToDo.wiki"
    else
      sh "svn commit wiki -m 'To Do list refresh'"
    end
  end
end
