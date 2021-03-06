unless FileTest.exists? 'Blocks.txt'
  unless system( "curl", "-s", "ftp://ftp.unicode.org/Public/5.1.0/ucd/Blocks.txt", "-o", "Blocks.txt" )
    STDERR.puts "Couldn't download Blocks.txt."
    exit(-1)
  end
end

class Integer
  def between_inclusive?( min, max )
    (self >= min) && (self <= max)
  end
end

# $pdfs_to_skip = Hash.new
#
# $pdfs_to_skip['U1FF80'] = 'Unassigned'
# $pdfs_to_skip['U2FF80'] = 'Unassigned'
# $pdfs_to_skip['U3FF80'] = 'Unassigned'
# $pdfs_to_skip['U4FF80'] = 'Unassigned'
# $pdfs_to_skip['U5FF80'] = 'Unassigned'
# $pdfs_to_skip['U6FF80'] = 'Unassigned'
# $pdfs_to_skip['U7FF80'] = 'Unassigned'
# $pdfs_to_skip['U8FF80'] = 'Unassigned'
# $pdfs_to_skip['U9FF80'] = 'Unassigned'
# $pdfs_to_skip['UAFF80'] = 'Unassigned'
# $pdfs_to_skip['UBFF80'] = 'Unassigned'
# $pdfs_to_skip['UCFF80'] = 'Unassigned'
# $pdfs_to_skip['UDFF80'] = 'Unassigned'
# $pdfs_to_skip['UEFF80'] = 'Unassigned'
# $pdfs_to_skip['UFFF80'] = 'Supplementary Private Use Area-B'
# $pdfs_to_skip['U10FF80'] = 'Supplementary Private Use Area-B'

class Block

  attr_accessor :first, :last, :name, :skip

  def initialize( first, last, name )
    @first = first
    @last = last
    @name = name
    @skip = false
    @first_i = Integer("0x"+first)
    @last_i = Integer("0x"+last)
  end

  def in_block?( code_point )
    code_point.between_inclusive?(@first_i,@last_i)
  end

  def nice_name
    "#{@name} (0x#{@first} to 0x#{@last})"
  end

end

$blocks = Array.new
$page_to_block = {}

open("Blocks.txt","r") do |f|
  f.each do |line|
    line.chomp!
    if line =~ /^([0-9A-F]+)\.\.([0-9A-F]+); (.*) *$/
      b = Block.new($1,$2,$3)
      $blocks.push b
      $page_to_block[$1] = b
    end
  end
end

def block_from_codepoint( codepoint )
  c = codepoint
  if c.class == String
    c = c.to_i(16)
  end
  $blocks.each do |b|
    if b.in_block? c
      return b
    end
  end
  raise "Filed to find block for codepoint #{codepoint}"
end

# This returns nil if we should skip it or a string with the name
# otherwise:

# def name_of_page( file_basename )
#
#   if $pdfs_to_skip.has_key? file_basename
#     return nil
#   end
#
#   code_point = Integer("0x"+file_basename.gsub(/^U/,''))
#
#   $blocks.each do |b|
#     if b.in_block? code_point
#       return b.name
#     end
#   end
#
#   nil
#
# end

class TileInfo

  attr_accessor :w, :h
  attr_accessor :filename
  attr_accessor :block
  attr_accessor :ocr_codepoint

  def initialize(w,h)
    @w = w
    @h = h
    @filename = nil
    @block = nil
    @ocr_codepoint = nil
  end

  def valid_size?
    (@w > 200 and @w < 500 and @h > 250 and @h < 370)
  end

  def apparently_successful_ocr?
    @ocr_codepoint and not (@ocr_codepoint.class == String)
  end

  def set_block_from_ocr_codepoint
    @block = block_from_codepoint(@ocr_codepoint)
  end

  def TileInfo.from_filename( filename )
    d = nil
    s = `png-find-grid/png-size #{filename}`.chomp
    unless $?.success?
      puts "png-find-grid/png-size #{filename} failed"
      exit(-1)
    end
    if s =~ /^(\d+)x(\d+)/
      d = TileInfo.new(Integer($1),Integer($2))
      d.filename = filename
      d.block = name_of_page filename.gsub(/^(U[0-9A-F]+).*$/,'\1')
    end
    return d
  end

  def to_s
    "#{filename}: #{w} x #{h} (in block '#{block}'"
  end

  def to_yaml_hash_element
    "#{filename}: !ruby/object:TileInfo\n"+
      "  filename: #{filename}\n"+
      "  h: #{h}\n"+
      "  w: #{w}\n"+
      "  block: #{block}\n"+
      "  ocr_codepoint: #{ocr_codepoint}"
  end

end
