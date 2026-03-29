# frozen_string_literal: true

# FWUI::Doc — document-oriented API with presets for academic documents.
#
#   docx = FWUI::Doc.to_docx(tree, preset: :gost)
#   odt  = FWUI::Doc.to_odt(tree, preset: :gost, header: "My Report")
module FWUI
  module Doc
    GOST = {
      "page_size" => "A4",
      "margin_top" => "20mm", "margin_bottom" => "20mm",
      "margin_left" => "30mm", "margin_right" => "15mm",
      "default_font" => "Times New Roman",
      "default_font_size" => "14pt",
      "line_spacing" => "1.5",
      "first_line_indent" => "1.25cm",
    }.freeze

    def self.page_break = FWUI.node("__page_break__")

    def self.to_docx(node, preset: nil, **opts)
      config = (preset ? const_get(preset.to_s.upcase) : {}).merge(string_keys(opts))
      NativeDocx.render_docx(node, config)
    end

    def self.to_odt(node, preset: nil, **opts)
      config = (preset ? const_get(preset.to_s.upcase) : {}).merge(string_keys(opts))
      NativeDocx.render_odt(node, config)
    end

    private_class_method def self.string_keys(h)
      h.transform_keys(&:to_s)
    end
  end
end
