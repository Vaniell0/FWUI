# frozen_string_literal: true

# DOCX/ODT export for FWUI nodes — native C++ rendering.
# Zero dependencies: ZIP + XML generated in C++.
module FWUI
  class Node
    # Render this node tree as a DOCX binary string.
    #   File.binwrite("out.docx", node.to_docx)
    def to_docx
      NativeDocx.render_docx(self)
    end
  end

  module Native
    # Render a node tree to DOCX format (binary string).
    #   docx = FWUI::Native.to_docx(node)
    #   File.binwrite("output.docx", docx)
    def self.to_docx(node)
      NativeDocx.render_docx(node)
    end
  end
end
