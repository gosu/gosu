require 'rubygems'
require 'mysql'
require 'RMagick'

begin
  # connect to the MySQL server
  dbh = Mysql.real_connect("localhost", ARGV[0], ARGV[1], "mwf")
  res = dbh.query %( select t.id, t.subject, a.postId, a.fileName
                     from topics t join attachments a on (t.basePostId = a.postId)
                     where boardId = (select id from boards where title = 'Gosu Showcase' and
                                       (a.filename like '%.png' or a.filename like '%.jpg')) )
  ary = []
  res.each { |row| ary << row }
  res.free

  # Some files may not exist thanks to earlier libgosu server failage.
  filename = "man_this_script_sucks"
  while not File.exist? filename do
    winner = ary[rand(ary.size)]
    filename = "mwf/attachments/#{winner[2].to_i % 100}/#{winner[2]}/#{winner[3]}"
  end

  File.open('potd_values.rb', 'w') do |file|
    file.puts "$POTD_ID = #{winner[0]}"
    file.puts "$POTD_TITLE = %(#{winner[1]})"
  end

  image = Magick::Image.read(filename)[0]
  image.change_geometry! '276x350>' do |cols, rows, img|
    img.resize! cols, rows
  end
  image.write 'potd.png'
ensure
  # disconnect from server
  dbh.close if dbh
end
