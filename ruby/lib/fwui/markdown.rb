# frozen_string_literal: true

# Minimal Markdown → HTML converter, pure Ruby, zero dependencies.
# Supports: headers, bold, italic, links, inline code, code blocks,
# unordered/ordered lists, horizontal rules, paragraphs.

module FWUI
  module Markdown
    module_function

    def to_html(text)
      lines = text.gsub("\r\n", "\n").split("\n")
      html = +""
      i = 0

      while i < lines.length
        line = lines[i]

        # Blank line
        if line.strip.empty?
          i += 1
          next
        end

        # Fenced code block (```)
        if line.strip.start_with?('```')
          lang = line.strip.sub('```', '').strip
          code_lines = []
          i += 1
          while i < lines.length && !lines[i].strip.start_with?('```')
            code_lines << escape(lines[i])
            i += 1
          end
          i += 1 # skip closing ```
          cls = lang.empty? ? '' : " class=\"language-#{escape(lang)}\""
          html << "<pre><code#{cls}>#{code_lines.join("\n")}</code></pre>\n"
          next
        end

        # Horizontal rule
        if line.strip.match?(/\A(-{3,}|\*{3,}|_{3,})\z/)
          html << "<hr>\n"
          i += 1
          next
        end

        # Headers
        if line =~ /\A(\#{1,6})\s+(.*)/
          level = $1.length
          content = inline($2)
          html << "<h#{level}>#{content}</h#{level}>\n"
          i += 1
          next
        end

        # Unordered list
        if line =~ /\A\s*[-*+]\s+(.*)/
          html << "<ul>\n"
          while i < lines.length && lines[i] =~ /\A\s*[-*+]\s+(.*)/
            html << "  <li>#{inline($1)}</li>\n"
            i += 1
          end
          html << "</ul>\n"
          next
        end

        # Ordered list
        if line =~ /\A\s*\d+\.\s+(.*)/
          html << "<ol>\n"
          while i < lines.length && lines[i] =~ /\A\s*\d+\.\s+(.*)/
            html << "  <li>#{inline($1)}</li>\n"
            i += 1
          end
          html << "</ol>\n"
          next
        end

        # Blockquote
        if line.start_with?('>')
          quote_lines = []
          while i < lines.length && lines[i].start_with?('>')
            quote_lines << lines[i].sub(/\A>\s?/, '')
            i += 1
          end
          html << "<blockquote>#{to_html(quote_lines.join("\n"))}</blockquote>\n"
          next
        end

        # Paragraph (collect consecutive non-empty lines)
        para_lines = []
        while i < lines.length && !lines[i].strip.empty? &&
              !lines[i].match?(/\A\#{1,6}\s/) &&
              !lines[i].match?(/\A\s*[-*+]\s/) &&
              !lines[i].match?(/\A\s*\d+\.\s/) &&
              !lines[i].start_with?('```') &&
              !lines[i].start_with?('>')  &&
              !lines[i].strip.match?(/\A(-{3,}|\*{3,}|_{3,})\z/)
          para_lines << lines[i]
          i += 1
        end
        unless para_lines.empty?
          html << "<p>#{inline(para_lines.join("\n"))}</p>\n"
        end
      end

      html
    end

    # Process inline formatting
    def inline(text)
      s = escape(text)

      # Inline code (must be before bold/italic to avoid conflicts)
      s = s.gsub(/`([^`]+)`/) { "<code>#{$1}</code>" }

      # Images: ![alt](src)
      s = s.gsub(/!\[([^\]]*)\]\(([^)]+)\)/) { "<img src=\"#{$2}\" alt=\"#{$1}\">" }

      # Links: [text](url)
      s = s.gsub(/\[([^\]]+)\]\(([^)]+)\)/) { "<a href=\"#{$2}\">#{$1}</a>" }

      # Bold: **text** or __text__
      s = s.gsub(/\*\*(.+?)\*\*/) { "<strong>#{$1}</strong>" }
      s = s.gsub(/__(.+?)__/) { "<strong>#{$1}</strong>" }

      # Italic: *text* or _text_
      s = s.gsub(/\*(.+?)\*/) { "<em>#{$1}</em>" }
      s = s.gsub(/(?<!\w)_(.+?)_(?!\w)/) { "<em>#{$1}</em>" }

      # Line break: two trailing spaces or backslash
      s = s.gsub(/  \n/, "<br>\n")
      s = s.gsub(/\\\n/, "<br>\n")

      s
    end

    def escape(text)
      text.to_s
        .gsub('&', '&amp;')
        .gsub('<', '&lt;')
        .gsub('>', '&gt;')
        .gsub('"', '&quot;')
    end
  end

  # DSL methods
  module_function

  def markdown(text)
    raw(Markdown.to_html(text))
  end

  def markdown_file(path)
    markdown(File.read(path))
  end

  # Re-define DSL to pick up new methods
  module DSL
    unless method_defined?(:markdown)
      define_method(:markdown) { |*a, **kw, &b| FWUI.send(:markdown, *a, **kw, &b) }
      define_method(:markdown_file) { |*a, **kw, &b| FWUI.send(:markdown_file, *a, **kw, &b) }
    end
  end
end
