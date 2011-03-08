GIT_PREFIX="PATH=/usr/local/git/bin:$PATH"

desc "Uses a parallel gosu.git repository and git-svn to push changes to github"
task :push_to_github do
  Dir.chdir '../gosu.git'
  fetch_output = `#{GIT_PREFIX} git svn fetch`
  puts fetch_output
  if not fetch_output.empty? then
    last_commit_id = fetch_output.reverse[/ [a-f0-9]{40} /].reverse
    sh "#{GIT_PREFIX} git rebase #{last_commit_id}"
    sh "#{GIT_PREFIX} git push origin master"
  end
end
