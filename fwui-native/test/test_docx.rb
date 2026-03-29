# frozen_string_literal: true

# Tests for DOCX/ODT export.
# Run: cd fwui-native && rake compile && ruby test/test_docx.rb

$LOAD_PATH.unshift File.expand_path('../../ruby/lib', __dir__)
$LOAD_PATH.unshift File.expand_path('../lib', __dir__)

require 'fwui'
require 'fwui/native'
require 'tempfile'

module DocxTests
  @passed = 0
  @failed = 0

  def self.assert_eq(desc, actual, expected)
    if actual == expected
      @passed += 1
    else
      @failed += 1
      $stderr.puts "  FAIL: #{desc}"
      $stderr.puts "    expected: #{expected.inspect[0..200]}"
      $stderr.puts "    actual:   #{actual.inspect[0..200]}"
    end
  end

  def self.assert(desc)
    if yield
      @passed += 1
    else
      @failed += 1
      $stderr.puts "  FAIL: #{desc}"
    end
  rescue => e
    @failed += 1
    $stderr.puts "  ERROR: #{desc} — #{e.message}"
  end

  def self.summary
    total = @passed + @failed
    if @failed > 0
      puts "\n#{total} tests, #{@failed} FAILED"
      exit 1
    else
      puts "#{total} tests, all passed"
    end
  end

  # Extract a named file from a ZIP binary string (STORE only)
  def self.zip_entry(zip_data, name)
    offset = 0
    while offset < zip_data.bytesize - 4
      sig = zip_data[offset, 4].unpack1('V')
      break if sig != 0x04034B50 # not a local file header

      name_len = zip_data[offset + 26, 2].unpack1('v')
      extra_len = zip_data[offset + 28, 2].unpack1('v')
      comp_size = zip_data[offset + 18, 4].unpack1('V')
      entry_name = zip_data[offset + 30, name_len]
      data_offset = offset + 30 + name_len + extra_len

      if entry_name == name
        return zip_data[data_offset, comp_size]
      end

      offset = data_offset + comp_size
    end
    nil
  end

  # Extract document.xml from DOCX binary (returns UTF-8 string)
  def self.doc_xml(docx)
    data = zip_entry(docx, 'word/document.xml')
    data&.force_encoding('UTF-8')
  end

  def self.run
    puts "=== DOCX/ODT export tests ==="

    test_docx_is_valid_zip
    test_docx_has_required_files
    test_docx_basic_paragraph
    test_docx_heading_levels
    test_docx_text_formatting
    test_docx_color
    test_docx_font_size
    test_docx_font_family
    test_docx_text_align
    test_docx_br
    test_docx_hr
    test_docx_containers
    test_docx_nested_inline
    test_docx_code_pre
    test_docx_margin_padding
    test_docx_xml_escaping
    test_docx_utf8
    test_docx_empty_tree
    test_docx_large_tree
    test_docx_node_method
    test_docx_native_method
    test_docx_writes_openable_file
    test_docx_unordered_list
    test_docx_ordered_list
    test_docx_nested_list
    test_docx_table
    test_docx_table_with_header
    test_docx_hyperlink
    test_docx_image

    summary
  end

  def self.test_docx_is_valid_zip
    puts "  valid ZIP..."
    docx = FWUI.p("Hello").to_docx
    assert("binary encoding") { docx.encoding == Encoding::ASCII_8BIT }
    assert("starts with PK") { docx[0, 2] == "PK" }
    assert("has end-of-central-dir") { docx.include?([0x06054B50].pack('V')) }
  end

  def self.test_docx_has_required_files
    puts "  required ZIP entries..."
    docx = FWUI.p("Hello").to_docx
    assert("has [Content_Types].xml") { zip_entry(docx, '[Content_Types].xml') }
    assert("has _rels/.rels") { zip_entry(docx, '_rels/.rels') }
    assert("has word/document.xml") { zip_entry(docx, 'word/document.xml') }
    assert("has word/styles.xml") { zip_entry(docx, 'word/styles.xml') }
    assert("has word/_rels/document.xml.rels") { zip_entry(docx, 'word/_rels/document.xml.rels') }
  end

  def self.test_docx_basic_paragraph
    puts "  basic paragraph..."
    xml = doc_xml(FWUI.p("Hello world").to_docx)
    assert("has w:p") { xml.include?('<w:p>') || xml.include?('<w:p ') }
    assert("has w:r") { xml.include?('<w:r>') }
    assert("has text") { xml.include?('Hello world') }
  end

  def self.test_docx_heading_levels
    puts "  heading levels..."
    (1..6).each do |n|
      node = FWUI.send("h#{n}", "Heading #{n}")
      xml = doc_xml(node.to_docx)
      assert("h#{n} has Heading#{n} style") { xml.include?("Heading#{n}") }
      assert("h#{n} has text") { xml.include?("Heading #{n}") }
    end
  end

  def self.test_docx_text_formatting
    puts "  text formatting..."

    xml = doc_xml(FWUI.node("p", children: [FWUI.strong("bold text")]).to_docx)
    assert("strong → w:b") { xml.include?('<w:b/>') }
    assert("strong has text") { xml.include?('bold text') }

    xml = doc_xml(FWUI.node("p", children: [FWUI.em("italic text")]).to_docx)
    assert("em → w:i") { xml.include?('<w:i/>') }

    xml = doc_xml(FWUI.p("underlined").set_style("text-decoration", "underline").to_docx)
    assert("underline → w:u") { xml.include?('w:u') }

    xml = doc_xml(FWUI.p("struck").set_style("text-decoration", "line-through").to_docx)
    assert("strikethrough → w:strike") { xml.include?('<w:strike/>') }

    # Bold via style
    xml = doc_xml(FWUI.p("style-bold").bold.to_docx)
    assert("bold style → w:b") { xml.include?('<w:b/>') }

    # Italic via style
    xml = doc_xml(FWUI.p("style-italic").italic.to_docx)
    assert("italic style → w:i") { xml.include?('<w:i/>') }
  end

  def self.test_docx_color
    puts "  color..."
    xml = doc_xml(FWUI.p("red text").color("red").to_docx)
    assert("named color → w:color") { xml.include?('w:color') && xml.include?('FF0000') }

    xml = doc_xml(FWUI.p("hex color").color("#336699").to_docx)
    assert("hex color → w:color") { xml.include?('336699') }

    xml = doc_xml(FWUI.p("bg").bg_color("#FFCC00").to_docx)
    assert("bg_color → w:shd") { xml.include?('w:shd') && xml.include?('FFCC00') }
  end

  def self.test_docx_font_size
    puts "  font size..."
    xml = doc_xml(FWUI.p("big").font_size("24pt").to_docx)
    # 24pt = 48 half-points
    assert("font-size → w:sz val=48") { xml.include?('w:sz') && xml.include?('"48"') }
  end

  def self.test_docx_font_family
    puts "  font family..."
    xml = doc_xml(FWUI.p("arial").set_style("font-family", "Arial").to_docx)
    assert("font-family → w:rFonts") { xml.include?('w:rFonts') && xml.include?('Arial') }
  end

  def self.test_docx_text_align
    puts "  text alignment..."
    xml = doc_xml(FWUI.p("centered").set_style("text-align", "center").to_docx)
    assert("text-align center → w:jc") { xml.include?('w:jc') && xml.include?('center') }

    xml = doc_xml(FWUI.p("right").set_style("text-align", "right").to_docx)
    assert("text-align right → w:jc end") { xml.include?('w:jc') && xml.include?('end') }
  end

  def self.test_docx_br
    puts "  line break..."
    xml = doc_xml(FWUI.node("p", children: [FWUI.text("line1"), FWUI.br, FWUI.text("line2")]).to_docx)
    assert("br → w:br") { xml.include?('<w:br/>') }
    assert("has both lines") { xml.include?('line1') && xml.include?('line2') }
  end

  def self.test_docx_hr
    puts "  horizontal rule..."
    xml = doc_xml(FWUI.div([FWUI.p("above"), FWUI.hr, FWUI.p("below")]).to_docx)
    assert("hr → pBdr bottom") { xml.include?('w:pBdr') && xml.include?('w:bottom') }
  end

  def self.test_docx_containers
    puts "  containers..."
    tree = FWUI.div([
      FWUI.section([
        FWUI.article([FWUI.p("Deep content")])
      ])
    ])
    xml = doc_xml(tree.to_docx)
    assert("container children rendered") { xml.include?('Deep content') }
  end

  def self.test_docx_nested_inline
    puts "  nested inline..."
    tree = FWUI.node("p", children: [
      FWUI.node("strong", children: [FWUI.em("bold-italic")])
    ])
    xml = doc_xml(tree.to_docx)
    assert("nested bold+italic") { xml.include?('<w:b/>') && xml.include?('<w:i/>') }
    assert("has text") { xml.include?('bold-italic') }
  end

  def self.test_docx_code_pre
    puts "  code/pre..."
    xml = doc_xml(FWUI.node("p", children: [FWUI.code("monospace")]).to_docx)
    assert("code → Courier New") { xml.include?('Courier New') }

    xml = doc_xml(FWUI.pre("preformatted").to_docx)
    assert("pre → Courier New") { xml.include?('Courier New') }
  end

  def self.test_docx_margin_padding
    puts "  margin/padding..."
    xml = doc_xml(FWUI.p("spaced").margin("20px").padding("10px").to_docx)
    assert("margin → w:spacing") { xml.include?('w:spacing') }
    assert("padding → w:ind") { xml.include?('w:ind') }
  end

  def self.test_docx_xml_escaping
    puts "  XML escaping..."
    xml = doc_xml(FWUI.p("A & B < C > D \"E\"").to_docx)
    assert("& escaped") { xml.include?('A &amp; B') }
    assert("< escaped") { xml.include?('&lt; C') }
    assert("> escaped") { xml.include?('&gt; D') }
  end

  def self.test_docx_utf8
    puts "  UTF-8..."
    xml = doc_xml(FWUI.p("Привет мир 🎉").to_docx)
    assert("cyrillic preserved") { xml.include?('Привет мир') }
    assert("emoji preserved") { xml.include?('🎉') }
  end

  def self.test_docx_empty_tree
    puts "  empty tree..."
    docx = FWUI.div.to_docx
    assert("empty div produces valid ZIP") { docx[0, 2] == "PK" }
    xml = doc_xml(docx)
    assert("has document xml") { xml && xml.include?('w:document') }
  end

  def self.test_docx_large_tree
    puts "  large tree (100 nodes)..."
    tree = FWUI.div((1..100).map { |i|
      FWUI.div([
        FWUI.h1("Item #{i}").bold.color("#333"),
        FWUI.p("Desc #{i}").padding("10px"),
      ])
    })
    docx = tree.to_docx
    assert("large tree valid ZIP") { docx[0, 2] == "PK" }
    xml = doc_xml(docx)
    assert("has first item") { xml.include?('Item 1') }
    assert("has last item") { xml.include?('Item 100') }
  end

  def self.test_docx_node_method
    puts "  Node#to_docx..."
    docx = FWUI.h1("Test").to_docx
    assert("returns binary string") { docx.is_a?(String) && docx.encoding == Encoding::ASCII_8BIT }
    assert("is a DOCX") { docx[0, 2] == "PK" }
  end

  def self.test_docx_native_method
    puts "  Native.to_docx..."
    docx = FWUI::Native.to_docx(FWUI.p("test"))
    assert("returns binary string") { docx.is_a?(String) }
    assert("is a DOCX") { docx[0, 2] == "PK" }
  end

  def self.test_docx_writes_openable_file
    puts "  write to file..."
    tree = FWUI.div([
      FWUI.h1("FWUI DOCX Export Test").bold.color("#333"),
      FWUI.p("This document was generated by fwui-native.").italic,
      FWUI.node("p", children: [
        FWUI.text("Mixed "),
        FWUI.strong("bold"),
        FWUI.text(" and "),
        FWUI.em("italic"),
        FWUI.text(" text."),
      ]),
      FWUI.h2("Code Example"),
      FWUI.pre("def hello\n  puts 'world'\nend"),
      FWUI.hr,
      FWUI.p("Footer text").color("gray"),
    ])

    f = Tempfile.new(['fwui_test', '.docx'])
    f.binmode
    f.write(tree.to_docx)
    f.close

    size = File.size(f.path)
    assert("file written (#{size} bytes)") { size > 100 }

    # Verify it's a valid ZIP by checking we can extract document.xml
    data = File.binread(f.path)
    xml = doc_xml(data)
    assert("file contains valid document.xml") { xml && xml.include?('w:document') }

    f.unlink
  end

  def self.test_docx_unordered_list
    puts "  unordered list..."
    tree = FWUI.ul([FWUI.li("Apple"), FWUI.li("Banana"), FWUI.li("Cherry")])
    docx = tree.to_docx
    xml = doc_xml(docx)
    assert("ul has w:numPr") { xml.include?('w:numPr') }
    assert("ul has all items") { xml.include?('Apple') && xml.include?('Banana') && xml.include?('Cherry') }

    # Check numbering.xml exists
    num_xml = zip_entry(docx, 'word/numbering.xml')&.force_encoding('UTF-8')
    assert("has numbering.xml") { num_xml }
    assert("has bullet format") { num_xml&.include?('bullet') }
  end

  def self.test_docx_ordered_list
    puts "  ordered list..."
    tree = FWUI.ol([FWUI.li("First"), FWUI.li("Second"), FWUI.li("Third")])
    docx = tree.to_docx
    xml = doc_xml(docx)
    assert("ol has w:numPr") { xml.include?('w:numPr') }
    assert("ol has items") { xml.include?('First') && xml.include?('Second') }

    num_xml = zip_entry(docx, 'word/numbering.xml')&.force_encoding('UTF-8')
    assert("has decimal format") { num_xml&.include?('decimal') }
  end

  def self.test_docx_nested_list
    puts "  nested list..."
    tree = FWUI.ul([
      FWUI.li("Parent"),
      FWUI.node("li", children: [
        FWUI.text("With sub"),
        FWUI.ul([FWUI.li("Child 1"), FWUI.li("Child 2")])
      ])
    ])
    docx = tree.to_docx
    xml = doc_xml(docx)
    assert("nested has all items") { xml.include?('Parent') && xml.include?('Child 1') && xml.include?('Child 2') }
    # Nested items should have ilvl=1
    assert("nested has level 1") { xml.include?('"1"') }
  end

  def self.test_docx_table
    puts "  table..."
    tree = FWUI.node("table", children: [
      FWUI.node("tr", children: [
        FWUI.node("td", text: "Cell 1"),
        FWUI.node("td", text: "Cell 2"),
      ]),
      FWUI.node("tr", children: [
        FWUI.node("td", text: "Cell 3"),
        FWUI.node("td", text: "Cell 4"),
      ]),
    ])
    xml = doc_xml(tree.to_docx)
    assert("has w:tbl") { xml.include?('w:tbl') }
    assert("has w:tr") { xml.include?('w:tr') }
    assert("has w:tc") { xml.include?('w:tc') }
    assert("has all cells") {
      xml.include?('Cell 1') && xml.include?('Cell 2') &&
      xml.include?('Cell 3') && xml.include?('Cell 4')
    }
    assert("has table borders") { xml.include?('w:tblBorders') }
  end

  def self.test_docx_table_with_header
    puts "  table with header..."
    tree = FWUI.node("table", children: [
      FWUI.node("tr", children: [
        FWUI.node("th", text: "Name"),
        FWUI.node("th", text: "Value"),
      ]),
      FWUI.node("tr", children: [
        FWUI.node("td", text: "foo"),
        FWUI.node("td", text: "bar"),
      ]),
    ])
    xml = doc_xml(tree.to_docx)
    assert("th has bold (w:b)") { xml.include?('<w:b/>') }
    assert("has header text") { xml.include?('Name') && xml.include?('Value') }
  end

  def self.test_docx_hyperlink
    puts "  hyperlink..."
    tree = FWUI.div([FWUI.a("Click here", href: "https://example.com")])
    docx = tree.to_docx
    xml = doc_xml(docx)
    assert("has w:hyperlink") { xml.include?('w:hyperlink') }
    assert("has link text") { xml.include?('Click here') }
    assert("has blue color") { xml.include?('0563C1') }

    # Check relationship
    rels_xml = zip_entry(docx, 'word/_rels/document.xml.rels')&.force_encoding('UTF-8')
    assert("has hyperlink relationship") { rels_xml&.include?('https://example.com') }
    assert("relationship is External") { rels_xml&.include?('External') }
  end

  def self.test_docx_image
    puts "  image..."
    # Create a minimal 1x1 PNG for testing
    png_data = [
      0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A, # PNG signature
      0x00, 0x00, 0x00, 0x0D, 0x49, 0x48, 0x44, 0x52, # IHDR chunk
      0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01,
      0x08, 0x02, 0x00, 0x00, 0x00, 0x90, 0x77, 0x53,
      0xDE, 0x00, 0x00, 0x00, 0x0C, 0x49, 0x44, 0x41, # IDAT chunk
      0x54, 0x08, 0xD7, 0x63, 0xF8, 0xCF, 0xC0, 0x00,
      0x00, 0x00, 0x02, 0x00, 0x01, 0xE2, 0x21, 0xBC,
      0x33, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4E, # IEND chunk
      0x44, 0xAE, 0x42, 0x60, 0x82,
    ].pack('C*')

    f = Tempfile.new(['test_img', '.png'])
    f.binmode
    f.write(png_data)
    f.close

    tree = FWUI.img(f.path, alt: "test image")
    docx = tree.to_docx
    xml = doc_xml(docx)
    assert("has w:drawing") { xml.include?('w:drawing') }
    assert("has wp:inline") { xml.include?('wp:inline') }
    assert("has pic:pic") { xml.include?('pic:pic') }

    # Check image is embedded in ZIP
    img_data = zip_entry(docx, 'word/media/image1.png')
    assert("image embedded in ZIP") { img_data && img_data.bytesize == png_data.bytesize }

    f.unlink
  end
end

DocxTests.run
