/*
 * fwui_docx.cpp — DOCX and ODT export for fwui-native.
 *
 * Zero dependencies beyond C++17 stdlib. ZIP uses STORE (no compression).
 * Walks FWUI::Node tree and emits OOXML / ODF XML.
 *
 * Provides:
 *   FWUI::NativeDocx.render_docx(node) → String (binary ZIP)
 *   FWUI::NativeDocx.render_odt(node)  → String (binary ZIP)
 */

#include <ruby.h>
#include <ruby/encoding.h>

#include <string>
#include <vector>
#include <cstring>
#include <cstdint>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <sstream>

/* ── Shared symbols from fwui_native.c ─────────────────────────── */

extern "C" {
    extern int direct_ok;
    extern ID id_tag, id_text, id_raw_html, id_attrs, id_styles;
    extern ID id_classes, id_id, id_children;
    extern VALUE sym_text_node;
    extern rb_encoding *enc_utf8;
}

/* ── CRC-32 (ISO 3309 polynomial) ─────────────────────────────── */

static uint32_t crc32_table[256];
static int crc32_ready = 0;

static void crc32_init() {
    if (crc32_ready) return;
    for (uint32_t i = 0; i < 256; i++) {
        uint32_t c = i;
        for (int j = 0; j < 8; j++)
            c = (c >> 1) ^ (c & 1 ? 0xEDB88320u : 0);
        crc32_table[i] = c;
    }
    crc32_ready = 1;
}

static uint32_t crc32_buf(const void *data, size_t len) {
    auto *p = static_cast<const uint8_t *>(data);
    uint32_t crc = 0xFFFFFFFFu;
    for (size_t i = 0; i < len; i++)
        crc = crc32_table[(crc ^ p[i]) & 0xFF] ^ (crc >> 8);
    return crc ^ 0xFFFFFFFFu;
}

/* ── XmlWriter ─────────────────────────────────────────────────── */

class XmlWriter {
    std::string buf_;
public:
    XmlWriter() { buf_.reserve(8192); }

    XmlWriter& decl() {
        buf_ += "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n";
        return *this;
    }

    XmlWriter& open(const char *tag) {
        buf_ += '<';
        buf_ += tag;
        buf_ += '>';
        return *this;
    }

    XmlWriter& open_attr(const char *tag) {
        buf_ += '<';
        buf_ += tag;
        return *this;
    }

    XmlWriter& attr(const char *name, const char *val) {
        buf_ += ' ';
        buf_ += name;
        buf_ += "=\"";
        xml_escape(val);
        buf_ += '"';
        return *this;
    }

    XmlWriter& attr(const char *name, const std::string &val) {
        return attr(name, val.c_str());
    }

    XmlWriter& attr(const char *name, int val) {
        buf_ += ' ';
        buf_ += name;
        buf_ += "=\"";
        buf_ += std::to_string(val);
        buf_ += '"';
        return *this;
    }

    XmlWriter& end_open() {
        buf_ += '>';
        return *this;
    }

    XmlWriter& close(const char *tag) {
        buf_ += "</";
        buf_ += tag;
        buf_ += '>';
        return *this;
    }

    XmlWriter& self_close() {
        buf_ += "/>";
        return *this;
    }

    XmlWriter& text(const char *s) {
        xml_escape(s);
        return *this;
    }

    XmlWriter& text(const std::string &s) {
        xml_escape(s.c_str(), s.size());
        return *this;
    }

    XmlWriter& raw(const char *s) {
        buf_ += s;
        return *this;
    }

    XmlWriter& raw(const std::string &s) {
        buf_ += s;
        return *this;
    }

    const std::string& str() const { return buf_; }
    void clear() { buf_.clear(); }

private:
    void xml_escape(const char *s) {
        for (; *s; s++) {
            switch (*s) {
            case '&':  buf_ += "&amp;"; break;
            case '<':  buf_ += "&lt;"; break;
            case '>':  buf_ += "&gt;"; break;
            case '"':  buf_ += "&quot;"; break;
            case '\'': buf_ += "&apos;"; break;
            default:   buf_ += *s;
            }
        }
    }

    void xml_escape(const char *s, size_t len) {
        for (size_t i = 0; i < len; i++) {
            switch (s[i]) {
            case '&':  buf_ += "&amp;"; break;
            case '<':  buf_ += "&lt;"; break;
            case '>':  buf_ += "&gt;"; break;
            case '"':  buf_ += "&quot;"; break;
            case '\'': buf_ += "&apos;"; break;
            default:   buf_ += s[i];
            }
        }
    }
};

/* ── ZipWriter (STORE only, no compression) ────────────────────── */

class ZipWriter {
    struct Entry {
        std::string name;
        std::string data;
        uint32_t crc;
        uint32_t offset;
    };
    std::vector<Entry> entries_;
    std::string out_;

    void write_u16(uint16_t v) {
        out_ += static_cast<char>(v & 0xFF);
        out_ += static_cast<char>((v >> 8) & 0xFF);
    }
    void write_u32(uint32_t v) {
        out_ += static_cast<char>(v & 0xFF);
        out_ += static_cast<char>((v >> 8) & 0xFF);
        out_ += static_cast<char>((v >> 16) & 0xFF);
        out_ += static_cast<char>((v >> 24) & 0xFF);
    }

public:
    void add(const std::string &name, const std::string &data) {
        Entry e;
        e.name = name;
        e.data = data;
        e.crc = crc32_buf(data.data(), data.size());
        e.offset = 0;
        entries_.push_back(std::move(e));
    }

    std::string finish() {
        out_.clear();
        out_.reserve(65536);

        /* Local file headers + data */
        for (auto &e : entries_) {
            e.offset = static_cast<uint32_t>(out_.size());
            /* Local file header signature */
            write_u32(0x04034B50);
            write_u16(20);          /* version needed */
            write_u16(0);           /* flags */
            write_u16(0);           /* compression: STORE */
            write_u16(0);           /* mod time */
            write_u16(0);           /* mod date */
            write_u32(e.crc);
            write_u32(static_cast<uint32_t>(e.data.size())); /* compressed */
            write_u32(static_cast<uint32_t>(e.data.size())); /* uncompressed */
            write_u16(static_cast<uint16_t>(e.name.size()));
            write_u16(0);           /* extra field length */
            out_ += e.name;
            out_ += e.data;
        }

        /* Central directory */
        uint32_t cd_offset = static_cast<uint32_t>(out_.size());
        for (auto &e : entries_) {
            write_u32(0x02014B50);  /* central dir signature */
            write_u16(20);          /* version made by */
            write_u16(20);          /* version needed */
            write_u16(0);           /* flags */
            write_u16(0);           /* compression: STORE */
            write_u16(0);           /* mod time */
            write_u16(0);           /* mod date */
            write_u32(e.crc);
            write_u32(static_cast<uint32_t>(e.data.size()));
            write_u32(static_cast<uint32_t>(e.data.size()));
            write_u16(static_cast<uint16_t>(e.name.size()));
            write_u16(0);           /* extra field length */
            write_u16(0);           /* comment length */
            write_u16(0);           /* disk number start */
            write_u16(0);           /* internal attrs */
            write_u32(0);           /* external attrs */
            write_u32(e.offset);    /* local header offset */
            out_ += e.name;
        }

        uint32_t cd_size = static_cast<uint32_t>(out_.size()) - cd_offset;

        /* End of central directory */
        write_u32(0x06054B50);
        write_u16(0);               /* disk number */
        write_u16(0);               /* disk with CD */
        write_u16(static_cast<uint16_t>(entries_.size()));
        write_u16(static_cast<uint16_t>(entries_.size()));
        write_u32(cd_size);
        write_u32(cd_offset);
        write_u16(0);               /* comment length */

        return std::move(out_);
    }
};

/* ── Node reading helpers ──────────────────────────────────────── */

struct NodeData {
    VALUE tag;
    VALUE text;
    VALUE raw_html;
    VALUE attrs;
    VALUE styles;
    VALUE classes;
    VALUE node_id;
    VALUE children;
};

static NodeData read_node(VALUE node) {
    NodeData d;
    if (direct_ok) {
        VALUE *ivs = ROBJECT_IVPTR(node);
        d.tag      = ivs[0];
        d.text     = ivs[1];
        d.raw_html = ivs[2];
        d.attrs    = ivs[3];
        d.styles   = ivs[4];
        d.classes  = ivs[5];
        d.node_id  = ivs[6];
        d.children = ivs[7];
    } else {
        d.tag      = rb_ivar_get(node, id_tag);
        d.text     = rb_ivar_get(node, id_text);
        d.raw_html = rb_ivar_get(node, id_raw_html);
        d.attrs    = rb_ivar_get(node, id_attrs);
        d.styles   = rb_ivar_get(node, id_styles);
        d.classes  = rb_ivar_get(node, id_classes);
        d.node_id  = rb_ivar_get(node, id_id);
        d.children = rb_ivar_get(node, id_children);
    }
    return d;
}

static std::string rb_str_to_std(VALUE v) {
    if (!RB_TYPE_P(v, T_STRING)) return {};
    return std::string(RSTRING_PTR(v), static_cast<size_t>(RSTRING_LEN(v)));
}

static std::string get_style(VALUE styles, const char *key) {
    if (!RB_TYPE_P(styles, T_HASH)) return {};
    VALUE v = rb_hash_aref(styles, rb_str_new_cstr(key));
    if (NIL_P(v)) return {};
    return rb_str_to_std(v);
}

static std::string get_attr(VALUE attrs, const char *key) {
    if (!RB_TYPE_P(attrs, T_HASH)) return {};
    VALUE v = rb_hash_aref(attrs, rb_str_new_cstr(key));
    if (NIL_P(v)) return {};
    return rb_str_to_std(v);
}

static std::string tag_str(VALUE tag) {
    if (tag == sym_text_node) return ":text_node";
    if (!RB_TYPE_P(tag, T_STRING)) return {};
    return std::string(RSTRING_PTR(tag), static_cast<size_t>(RSTRING_LEN(tag)));
}

/* ── Color parsing: "#RRGGBB" / "rgb(...)" / named → "RRGGBB" ── */

static const std::unordered_map<std::string, std::string> named_colors = {
    {"red","FF0000"},{"green","008000"},{"blue","0000FF"},{"white","FFFFFF"},
    {"black","000000"},{"yellow","FFFF00"},{"orange","FFA500"},{"purple","800080"},
    {"pink","FFC0CB"},{"gray","808080"},{"grey","808080"},{"cyan","00FFFF"},
    {"magenta","FF00FF"},{"brown","A52A2A"},{"navy","000080"},{"teal","008080"},
    {"maroon","800000"},{"olive","808000"},{"lime","00FF00"},{"aqua","00FFFF"},
    {"silver","C0C0C0"},{"fuchsia","FF00FF"},
};

static std::string parse_color_to_hex6(const std::string &c) {
    if (c.empty()) return {};
    if (c[0] == '#') {
        std::string h = c.substr(1);
        if (h.size() == 3) {
            std::string out;
            for (char ch : h) { out += ch; out += ch; }
            return out;
        }
        if (h.size() == 6) return h;
        return {};
    }
    /* named color */
    std::string lower = c;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    auto it = named_colors.find(lower);
    if (it != named_colors.end()) return it->second;
    return {};
}

/* ── CSS unit parsing ──────────────────────────────────────────── */

/* Parse "12px", "10pt", "1.5em" → twips (1 twip = 1/1440 inch) */
static int css_to_twips(const std::string &val) {
    if (val.empty()) return 0;
    double num = 0;
    try { num = std::stod(val); } catch (...) { return 0; }
    if (val.find("pt") != std::string::npos) return static_cast<int>(num * 20);
    if (val.find("px") != std::string::npos) return static_cast<int>(num * 15);
    if (val.find("em") != std::string::npos) return static_cast<int>(num * 240);
    if (val.find("cm") != std::string::npos) return static_cast<int>(num * 567);
    if (val.find("mm") != std::string::npos) return static_cast<int>(num * 56.7);
    if (val.find("in") != std::string::npos) return static_cast<int>(num * 1440);
    /* default: assume px */
    return static_cast<int>(num * 15);
}

/* Parse to half-points (for w:sz) */
static int css_to_half_pt(const std::string &val) {
    if (val.empty()) return 0;
    double num = 0;
    try { num = std::stod(val); } catch (...) { return 0; }
    if (val.find("pt") != std::string::npos) return static_cast<int>(num * 2);
    if (val.find("px") != std::string::npos) return static_cast<int>(num * 1.5);
    if (val.find("em") != std::string::npos) return static_cast<int>(num * 24);
    /* default: assume pt */
    return static_cast<int>(num * 2);
}

/* ── Tag classification ────────────────────────────────────────── */

static bool is_heading(const std::string &tag) {
    return tag.size() == 2 && tag[0] == 'h' && tag[1] >= '1' && tag[1] <= '6';
}

static int heading_level(const std::string &tag) {
    return tag[1] - '0';
}

static bool is_block(const std::string &tag) {
    static const std::unordered_set<std::string> blocks = {
        "div","section","article","main","aside","header","footer","nav",
        "p","h1","h2","h3","h4","h5","h6","pre","blockquote","figure",
        "figcaption","details","summary","address"
    };
    return blocks.count(tag) > 0;
}

static bool is_container(const std::string &tag) {
    static const std::unordered_set<std::string> containers = {
        "div","section","article","main","aside","header","footer","nav",
        "figure","figcaption","details","summary","address","blockquote",
        "form","fieldset","dd","dt","dl"
    };
    return containers.count(tag) > 0;
}

static bool is_inline_format(const std::string &tag) {
    static const std::unordered_set<std::string> inlines = {
        "strong","b","em","i","u","s","del","strike","span","code",
        "sub","sup","small","mark","abbr","cite","dfn","kbd","samp","var","q"
    };
    return inlines.count(tag) > 0;
}

static bool is_skip_tag(const std::string &tag) {
    static const std::unordered_set<std::string> skips = {
        "script","style","svg","canvas","video","audio","iframe",
        "noscript","template","slot"
    };
    return skips.count(tag) > 0;
}

/* ══════════════════════════════════════════════════════════════════
 *  DOCX Renderer
 * ══════════════════════════════════════════════════════════════════ */

/* Run properties collected from inline ancestors */
struct RunProps {
    bool bold = false;
    bool italic = false;
    bool underline = false;
    bool strike = false;
    bool monospace = false;
    std::string color;      /* "RRGGBB" */
    std::string bg_color;   /* "RRGGBB" */
    std::string font_size;  /* half-points as string */
    std::string font_family;
    std::string letter_spacing; /* twips as string */
};

/* Paragraph properties from the block element */
struct ParaProps {
    std::string alignment;       /* left/center/right/justify */
    std::string spacing_before;  /* twips */
    std::string spacing_after;   /* twips */
    std::string line_spacing;    /* twips (line height) */
    std::string indent_left;     /* twips */
    std::string indent_right;    /* twips */
    int heading_level = 0;       /* 1-6 or 0 */
};

class DocxRenderer {
    XmlWriter body_;
    int rel_id_ = 1;
    std::vector<std::pair<std::string, std::string>> hyperlinks_; /* rId, url */
    std::vector<std::pair<std::string, std::string>> images_;     /* rId, path in zip */
    std::vector<std::string> image_data_;  /* binary data for embedded images */

    /* numbering state */
    int next_num_id_ = 1;
    struct NumDef { int num_id; bool ordered; };
    std::vector<NumDef> num_defs_;

    /* List nesting depth for numbering */
    struct ListCtx { int num_id; int level; bool ordered; };
    std::vector<ListCtx> list_stack_;

    /* Track state for inline content */
    bool in_paragraph_ = false;

    std::string next_rel_id() {
        return "rId" + std::to_string(rel_id_++);
    }

public:
    std::string render(VALUE root) {
        rel_id_ = 10; /* reserve low IDs for standard rels */
        walk(root, RunProps{}, ParaProps{});

        ZipWriter zip;
        zip.add("[Content_Types].xml", content_types());
        zip.add("_rels/.rels", top_rels());
        zip.add("word/_rels/document.xml.rels", doc_rels());
        zip.add("word/document.xml", document_xml());
        zip.add("word/styles.xml", styles_xml());

        if (!num_defs_.empty())
            zip.add("word/numbering.xml", numbering_xml());

        /* Embedded images */
        for (size_t i = 0; i < images_.size(); i++)
            zip.add(images_[i].second, image_data_[i]);

        return zip.finish();
    }

private:
    /* ── Run properties → OOXML ────────────────────────────────── */

    void write_rpr(XmlWriter &w, const RunProps &rp) {
        bool has_props = rp.bold || rp.italic || rp.underline || rp.strike ||
                         rp.monospace || !rp.color.empty() || !rp.bg_color.empty() ||
                         !rp.font_size.empty() || !rp.font_family.empty() ||
                         !rp.letter_spacing.empty();
        if (!has_props) return;

        w.open("w:rPr");
        if (rp.monospace || !rp.font_family.empty()) {
            std::string font = rp.font_family.empty() ? "Courier New" : rp.font_family;
            w.open_attr("w:rFonts").attr("w:ascii", font).attr("w:hAnsi", font).self_close();
        }
        if (rp.bold)      w.raw("<w:b/>");
        if (rp.italic)    w.raw("<w:i/>");
        if (rp.underline) w.raw("<w:u w:val=\"single\"/>");
        if (rp.strike)    w.raw("<w:strike/>");
        if (!rp.color.empty())
            w.open_attr("w:color").attr("w:val", rp.color).self_close();
        if (!rp.bg_color.empty())
            w.open_attr("w:shd").attr("w:val", "clear")
             .attr("w:color", "auto").attr("w:fill", rp.bg_color).self_close();
        if (!rp.font_size.empty())
            w.open_attr("w:sz").attr("w:val", rp.font_size).self_close();
        if (!rp.letter_spacing.empty())
            w.open_attr("w:spacing").attr("w:val", rp.letter_spacing).self_close();
        w.close("w:rPr");
    }

    /* ── Paragraph properties → OOXML ─────────────────────────── */

    void write_ppr(XmlWriter &w, const ParaProps &pp, int list_num_id = 0, int list_level = -1) {
        bool has_props = pp.heading_level > 0 || !pp.alignment.empty() ||
                         !pp.spacing_before.empty() || !pp.spacing_after.empty() ||
                         !pp.line_spacing.empty() || !pp.indent_left.empty() ||
                         !pp.indent_right.empty() || list_num_id > 0;
        if (!has_props) return;

        w.open("w:pPr");
        if (list_num_id > 0) {
            w.open("w:numPr");
            w.open_attr("w:ilvl").attr("w:val", list_level >= 0 ? list_level : 0).self_close();
            w.open_attr("w:numId").attr("w:val", list_num_id).self_close();
            w.close("w:numPr");
        }
        if (pp.heading_level > 0) {
            std::string style = "Heading" + std::to_string(pp.heading_level);
            w.open_attr("w:pStyle").attr("w:val", style).self_close();
        }
        if (!pp.alignment.empty()) {
            std::string jc = pp.alignment;
            if (jc == "left") jc = "start";
            else if (jc == "right") jc = "end";
            w.open_attr("w:jc").attr("w:val", jc).self_close();
        }
        if (!pp.spacing_before.empty() || !pp.spacing_after.empty() || !pp.line_spacing.empty()) {
            w.open_attr("w:spacing");
            if (!pp.spacing_before.empty()) w.attr("w:before", pp.spacing_before);
            if (!pp.spacing_after.empty())  w.attr("w:after", pp.spacing_after);
            if (!pp.line_spacing.empty())   w.attr("w:line", pp.line_spacing);
            w.self_close();
        }
        if (!pp.indent_left.empty() || !pp.indent_right.empty()) {
            w.open_attr("w:ind");
            if (!pp.indent_left.empty())  w.attr("w:left", pp.indent_left);
            if (!pp.indent_right.empty()) w.attr("w:right", pp.indent_right);
            w.self_close();
        }
        w.close("w:pPr");
    }

    /* ── Emit a text run ───────────────────────────────────────── */

    void emit_run(const std::string &text, const RunProps &rp) {
        body_.open("w:r");
        write_rpr(body_, rp);
        body_.open_attr("w:t").attr("xml:space", "preserve").end_open();
        body_.text(text);
        body_.close("w:t");
        body_.close("w:r");
    }

    /* ── Ensure we're inside a paragraph ───────────────────────── */

    void ensure_paragraph(const ParaProps &pp) {
        if (!in_paragraph_) {
            body_.open("w:p");
            write_ppr(body_, pp);
            in_paragraph_ = true;
        }
    }

    void close_paragraph() {
        if (in_paragraph_) {
            body_.close("w:p");
            in_paragraph_ = false;
        }
    }

    /* ── Table rendering ─────────────────────────────────────── */

    void walk_table(const NodeData &table_d, const RunProps &rp) {
        body_.open("w:tbl");

        /* Table properties: borders + auto width */
        body_.open("w:tblPr");
        body_.open("w:tblBorders");
        const char *border_types[] = {"w:top","w:left","w:bottom","w:right","w:insideH","w:insideV"};
        for (auto bt : border_types) {
            body_.open_attr(bt).attr("w:val","single").attr("w:sz",4)
                 .attr("w:space",0).attr("w:color","auto").self_close();
        }
        body_.close("w:tblBorders");
        body_.open_attr("w:tblW").attr("w:w",0).attr("w:type","auto").self_close();
        body_.close("w:tblPr");

        /* Walk rows — handle thead/tbody/tfoot transparently */
        if (RB_TYPE_P(table_d.children, T_ARRAY)) {
            long len = RARRAY_LEN(table_d.children);
            for (long i = 0; i < len; i++) {
                VALUE child = RARRAY_AREF(table_d.children, i);
                NodeData cd = read_node(child);
                std::string ctag = tag_str(cd.tag);
                if (ctag == "tr") {
                    walk_table_row(cd, rp);
                } else if (ctag == "thead" || ctag == "tbody" || ctag == "tfoot") {
                    if (RB_TYPE_P(cd.children, T_ARRAY)) {
                        long clen = RARRAY_LEN(cd.children);
                        for (long j = 0; j < clen; j++) {
                            VALUE row = RARRAY_AREF(cd.children, j);
                            NodeData rd = read_node(row);
                            if (tag_str(rd.tag) == "tr")
                                walk_table_row(rd, rp);
                        }
                    }
                }
            }
        }

        body_.close("w:tbl");
    }

    void walk_table_row(const NodeData &row_d, const RunProps &rp) {
        body_.open("w:tr");
        if (RB_TYPE_P(row_d.children, T_ARRAY)) {
            long len = RARRAY_LEN(row_d.children);
            for (long i = 0; i < len; i++) {
                VALUE child = RARRAY_AREF(row_d.children, i);
                NodeData cd = read_node(child);
                std::string ctag = tag_str(cd.tag);
                if (ctag == "td" || ctag == "th") {
                    walk_table_cell(cd, rp, ctag == "th");
                }
            }
        }
        body_.close("w:tr");
    }

    void walk_table_cell(const NodeData &cell_d, const RunProps &rp, bool is_header) {
        body_.open("w:tc");

        /* Cell properties */
        body_.open("w:tcPr");
        body_.open_attr("w:tcBorders").end_open();
        const char *border_types[] = {"w:top","w:left","w:bottom","w:right"};
        for (auto bt : border_types) {
            body_.open_attr(bt).attr("w:val","single").attr("w:sz",4)
                 .attr("w:space",0).attr("w:color","auto").self_close();
        }
        body_.close("w:tcBorders");

        /* Width from style */
        std::string width = get_style(cell_d.styles, "width");
        if (!width.empty()) {
            int tw = css_to_twips(width);
            if (tw > 0)
                body_.open_attr("w:tcW").attr("w:w",tw).attr("w:type","dxa").self_close();
        }

        body_.close("w:tcPr");

        /* Cell content — must contain at least one paragraph */
        RunProps cell_rp = merge_run_props(rp, cell_d.styles, is_header ? "th" : "td");
        if (is_header) cell_rp.bold = true;

        ParaProps cell_pp = make_para_props(cell_d.styles, is_header ? "th" : "td");

        bool had_content = false;
        bool saved_in_para = in_paragraph_;
        in_paragraph_ = false;

        if (RTEST(cell_d.text)) {
            ensure_paragraph(cell_pp);
            emit_run(rb_str_to_std(cell_d.text), cell_rp);
            had_content = true;
        }

        if (RB_TYPE_P(cell_d.children, T_ARRAY)) {
            long len = RARRAY_LEN(cell_d.children);
            for (long i = 0; i < len; i++) {
                walk(RARRAY_AREF(cell_d.children, i), cell_rp, cell_pp);
                had_content = true;
            }
        }

        close_paragraph();

        /* OOXML requires at least one w:p in every cell */
        if (!had_content)
            body_.raw("<w:p/>");

        body_.close("w:tc");
        in_paragraph_ = saved_in_para;
    }

    /* ── Image embedding ──────────────────────────────────────── */

    void emit_image(const std::string &src, VALUE attrs) {
        /* Try to read file from disk */
        std::string data;
        FILE *f = fopen(src.c_str(), "rb");
        if (f) {
            char buf[8192];
            size_t n;
            while ((n = fread(buf, 1, sizeof(buf), f)) > 0)
                data.append(buf, n);
            fclose(f);
        }

        if (data.empty()) return; /* skip if can't read */

        /* Determine extension */
        std::string ext = "png";
        if (src.size() > 4) {
            std::string tail = src.substr(src.size() - 4);
            std::transform(tail.begin(), tail.end(), tail.begin(), ::tolower);
            if (tail == ".jpg" || tail == "jpeg") ext = "jpeg";
            else if (tail == ".gif") ext = "gif";
            else if (tail == ".bmp") ext = "bmp";
        }

        std::string rid = next_rel_id();
        std::string zip_path = "word/media/image" + std::to_string(images_.size() + 1) + "." + ext;
        images_.push_back({rid, zip_path});
        image_data_.push_back(std::move(data));

        /* Default dimensions — EMU (1 inch = 914400 EMU) */
        int cx = 914400 * 4; /* 4 inches */
        int cy = 914400 * 3; /* 3 inches */

        /* Check for width/height attrs */
        std::string w_str = get_attr(attrs, "width");
        std::string h_str = get_attr(attrs, "height");
        if (!w_str.empty()) {
            try { cx = static_cast<int>(std::stod(w_str) * 9525); } catch (...) {}
        }
        if (!h_str.empty()) {
            try { cy = static_cast<int>(std::stod(h_str) * 9525); } catch (...) {}
        }

        /* w:drawing → wp:inline → a:graphic → pic:pic */
        body_.open("w:r");
        body_.open("w:drawing");
        body_.open_attr("wp:inline")
             .attr("distT",0).attr("distB",0).attr("distL",0).attr("distR",0)
             .end_open();
        body_.open_attr("wp:extent").attr("cx",cx).attr("cy",cy).self_close();
        body_.open("wp:docPr");
        body_.close("wp:docPr");
        body_.open("a:graphic");
        body_.open_attr("a:graphicData")
             .attr("uri","http://schemas.openxmlformats.org/drawingml/2006/picture")
             .end_open();
        body_.open("pic:pic");
        body_.open("pic:nvPicPr");
        body_.open("pic:cNvPr");
        body_.close("pic:cNvPr");
        body_.open("pic:cNvPicPr");
        body_.close("pic:cNvPicPr");
        body_.close("pic:nvPicPr");
        body_.open("pic:blipFill");
        body_.open_attr("a:blip").attr("r:embed",rid).self_close();
        body_.open("a:stretch");
        body_.raw("<a:fillRect/>");
        body_.close("a:stretch");
        body_.close("pic:blipFill");
        body_.open("pic:spPr");
        body_.open("a:xfrm");
        body_.open_attr("a:off").attr("x",0).attr("y",0).self_close();
        body_.open_attr("a:ext").attr("cx",cx).attr("cy",cy).self_close();
        body_.close("a:xfrm");
        body_.open_attr("a:prstGeom").attr("prst","rect").end_open();
        body_.raw("<a:avLst/>");
        body_.close("a:prstGeom");
        body_.close("pic:spPr");
        body_.close("pic:pic");
        body_.close("a:graphicData");
        body_.close("a:graphic");
        body_.close("wp:inline");
        body_.close("w:drawing");
        body_.close("w:r");
    }

    /* ── Collect run props from styles ─────────────────────────── */

    RunProps merge_run_props(const RunProps &parent, VALUE styles, const std::string &tag) {
        RunProps rp = parent;

        /* Tag-based formatting */
        if (tag == "strong" || tag == "b") rp.bold = true;
        if (tag == "em" || tag == "i")     rp.italic = true;
        if (tag == "u")                    rp.underline = true;
        if (tag == "s" || tag == "del" || tag == "strike") rp.strike = true;
        if (tag == "code" || tag == "pre" || tag == "kbd" || tag == "samp") rp.monospace = true;

        /* Style-based formatting */
        std::string fw = get_style(styles, "font-weight");
        if (fw == "bold" || fw == "700" || fw == "800" || fw == "900") rp.bold = true;

        std::string fs = get_style(styles, "font-style");
        if (fs == "italic" || fs == "oblique") rp.italic = true;

        std::string td = get_style(styles, "text-decoration");
        if (td.find("underline") != std::string::npos) rp.underline = true;
        if (td.find("line-through") != std::string::npos) rp.strike = true;

        std::string color = get_style(styles, "color");
        std::string hex = parse_color_to_hex6(color);
        if (!hex.empty()) rp.color = hex;

        std::string bg = get_style(styles, "background-color");
        hex = parse_color_to_hex6(bg);
        if (!hex.empty()) rp.bg_color = hex;

        std::string fsize = get_style(styles, "font-size");
        if (!fsize.empty()) {
            int hp = css_to_half_pt(fsize);
            if (hp > 0) rp.font_size = std::to_string(hp);
        }

        std::string ff = get_style(styles, "font-family");
        if (!ff.empty()) rp.font_family = ff;

        std::string ls = get_style(styles, "letter-spacing");
        if (!ls.empty()) {
            int tw = css_to_twips(ls);
            if (tw != 0) rp.letter_spacing = std::to_string(tw);
        }

        return rp;
    }

    /* ── Collect paragraph props from styles ───────────────────── */

    ParaProps make_para_props(VALUE styles, const std::string &tag) {
        ParaProps pp;

        if (is_heading(tag))
            pp.heading_level = heading_level(tag);

        std::string ta = get_style(styles, "text-align");
        if (!ta.empty()) pp.alignment = ta;

        /* Margins → spacing before/after */
        std::string mt = get_style(styles, "margin-top");
        if (!mt.empty()) pp.spacing_before = std::to_string(css_to_twips(mt));
        std::string mb = get_style(styles, "margin-bottom");
        if (!mb.empty()) pp.spacing_after = std::to_string(css_to_twips(mb));
        std::string m = get_style(styles, "margin");
        if (!m.empty()) {
            int tw = css_to_twips(m);
            std::string s = std::to_string(tw);
            if (pp.spacing_before.empty()) pp.spacing_before = s;
            if (pp.spacing_after.empty())  pp.spacing_after = s;
        }

        /* Padding → indentation */
        std::string pl = get_style(styles, "padding-left");
        if (!pl.empty()) pp.indent_left = std::to_string(css_to_twips(pl));
        std::string pr = get_style(styles, "padding-right");
        if (!pr.empty()) pp.indent_right = std::to_string(css_to_twips(pr));
        std::string p = get_style(styles, "padding");
        if (!p.empty()) {
            int tw = css_to_twips(p);
            std::string s = std::to_string(tw);
            if (pp.indent_left.empty())  pp.indent_left = s;
            if (pp.indent_right.empty()) pp.indent_right = s;
        }

        /* Line height */
        std::string lh = get_style(styles, "line-height");
        if (!lh.empty()) pp.line_spacing = std::to_string(css_to_twips(lh));

        return pp;
    }

    /* ── Main recursive walker ─────────────────────────────────── */

    void walk(VALUE node, const RunProps &rp, const ParaProps &pp) {
        NodeData d = read_node(node);

        /* Raw HTML — skip in document export */
        if (RTEST(d.raw_html)) return;

        std::string tag = tag_str(d.tag);

        /* Text node */
        if (tag == ":text_node") {
            if (RTEST(d.text)) {
                ensure_paragraph(pp);
                emit_run(rb_str_to_std(d.text), rp);
            }
            return;
        }

        /* Skip web-only tags */
        if (is_skip_tag(tag)) return;

        /* Merge run props from this element */
        RunProps child_rp = merge_run_props(rp, d.styles, tag);

        /* ── br ────────────────────────────────────────────────── */
        if (tag == "br") {
            ensure_paragraph(pp);
            body_.open("w:r");
            body_.raw("<w:br/>");
            body_.close("w:r");
            return;
        }

        /* ── hr ────────────────────────────────────────────────── */
        if (tag == "hr") {
            close_paragraph();
            body_.open("w:p");
            body_.open("w:pPr");
            body_.open("w:pBdr");
            body_.open_attr("w:bottom").attr("w:val", "single")
                 .attr("w:sz", 6).attr("w:space", 1)
                 .attr("w:color", "auto").self_close();
            body_.close("w:pBdr");
            body_.close("w:pPr");
            body_.close("w:p");
            return;
        }

        /* ── Heading or paragraph ──────────────────────────────── */
        if (is_heading(tag) || tag == "p" || tag == "pre") {
            close_paragraph();
            ParaProps child_pp = make_para_props(d.styles, tag);
            body_.open("w:p");
            write_ppr(body_, child_pp);
            in_paragraph_ = true;

            /* Emit text content */
            if (RTEST(d.text))
                emit_run(rb_str_to_std(d.text), child_rp);

            /* Walk children */
            if (RB_TYPE_P(d.children, T_ARRAY)) {
                long len = RARRAY_LEN(d.children);
                for (long i = 0; i < len; i++)
                    walk(RARRAY_AREF(d.children, i), child_rp, child_pp);
            }

            close_paragraph();
            return;
        }

        /* ── Container elements (div, section, etc.) ───────────── */
        if (is_container(tag)) {
            close_paragraph();
            ParaProps child_pp = make_para_props(d.styles, tag);

            if (RTEST(d.text)) {
                ensure_paragraph(child_pp);
                emit_run(rb_str_to_std(d.text), child_rp);
                close_paragraph();
            }

            if (RB_TYPE_P(d.children, T_ARRAY)) {
                long len = RARRAY_LEN(d.children);
                for (long i = 0; i < len; i++)
                    walk(RARRAY_AREF(d.children, i), child_rp, child_pp);
            }

            close_paragraph();
            return;
        }

        /* ── Inline formatting (strong, em, span, code, etc.) ── */
        if (is_inline_format(tag)) {
            if (RTEST(d.text)) {
                ensure_paragraph(pp);
                emit_run(rb_str_to_std(d.text), child_rp);
            }

            if (RB_TYPE_P(d.children, T_ARRAY)) {
                long len = RARRAY_LEN(d.children);
                for (long i = 0; i < len; i++)
                    walk(RARRAY_AREF(d.children, i), child_rp, pp);
            }
            return;
        }

        /* ── Lists (ul/ol) ─────────────────────────────────────── */
        if (tag == "ul" || tag == "ol") {
            close_paragraph();
            bool ordered = (tag == "ol");

            int level = 0;
            int num_id;
            if (list_stack_.empty()) {
                /* New top-level list — create numbering definition */
                num_id = next_num_id_++;
                num_defs_.push_back({num_id, ordered});
            } else {
                /* Nested list — reuse parent num_id, bump level */
                num_id = list_stack_.back().num_id;
                level = list_stack_.back().level + 1;
            }

            list_stack_.push_back({num_id, level, ordered});
            if (RB_TYPE_P(d.children, T_ARRAY)) {
                long len = RARRAY_LEN(d.children);
                for (long i = 0; i < len; i++)
                    walk(RARRAY_AREF(d.children, i), child_rp, pp);
            }
            list_stack_.pop_back();
            return;
        }

        /* ── List item ─────────────────────────────────────────── */
        if (tag == "li") {
            close_paragraph();
            ParaProps li_pp = make_para_props(d.styles, tag);

            int num_id = 0, level = 0;
            if (!list_stack_.empty()) {
                num_id = list_stack_.back().num_id;
                level = list_stack_.back().level;
            }

            body_.open("w:p");
            write_ppr(body_, li_pp, num_id, level);
            in_paragraph_ = true;

            if (RTEST(d.text))
                emit_run(rb_str_to_std(d.text), child_rp);

            if (RB_TYPE_P(d.children, T_ARRAY)) {
                long len = RARRAY_LEN(d.children);
                for (long i = 0; i < len; i++) {
                    VALUE child = RARRAY_AREF(d.children, i);
                    NodeData cd = read_node(child);
                    std::string ctag = tag_str(cd.tag);
                    /* Nested list inside li — close paragraph first */
                    if (ctag == "ul" || ctag == "ol") {
                        close_paragraph();
                        walk(child, child_rp, li_pp);
                        /* Re-open paragraph if more siblings follow? No — OOXML lists are flat paragraphs */
                    } else {
                        walk(child, child_rp, li_pp);
                    }
                }
            }

            close_paragraph();
            return;
        }

        /* ── Table ──────────────────────────────────────────────── */
        if (tag == "table") {
            close_paragraph();
            walk_table(d, child_rp);
            return;
        }
        /* thead/tbody/tfoot — transparent wrappers */
        if (tag == "thead" || tag == "tbody" || tag == "tfoot") {
            if (RB_TYPE_P(d.children, T_ARRAY)) {
                long len = RARRAY_LEN(d.children);
                for (long i = 0; i < len; i++)
                    walk(RARRAY_AREF(d.children, i), child_rp, pp);
            }
            return;
        }

        /* ── Hyperlinks ─────────────────────────────────────────── */
        if (tag == "a") {
            std::string href = get_attr(d.attrs, "href");
            ensure_paragraph(pp);

            /* Hyperlink run props: inherit parent + blue underline */
            RunProps link_rp = child_rp;
            if (link_rp.color.empty()) link_rp.color = "0563C1";
            link_rp.underline = true;

            if (!href.empty()) {
                std::string rid = next_rel_id();
                hyperlinks_.push_back({rid, href});
                body_.open_attr("w:hyperlink").attr("r:id", rid).end_open();
            }

            if (RTEST(d.text))
                emit_run(rb_str_to_std(d.text), link_rp);

            if (RB_TYPE_P(d.children, T_ARRAY)) {
                long len = RARRAY_LEN(d.children);
                for (long i = 0; i < len; i++)
                    walk(RARRAY_AREF(d.children, i), link_rp, pp);
            }

            if (!href.empty())
                body_.close("w:hyperlink");
            return;
        }

        /* ── Images ────────────────────────────────────────────── */
        if (tag == "img") {
            std::string src = get_attr(d.attrs, "src");
            if (src.empty()) return;

            ensure_paragraph(pp);
            emit_image(src, d.attrs);
            return;
        }

        /* ── Fallback: treat as container ──────────────────────── */
        if (RTEST(d.text)) {
            ensure_paragraph(pp);
            emit_run(rb_str_to_std(d.text), child_rp);
        }
        if (RB_TYPE_P(d.children, T_ARRAY)) {
            long len = RARRAY_LEN(d.children);
            for (long i = 0; i < len; i++)
                walk(RARRAY_AREF(d.children, i), child_rp, pp);
        }
    }

    /* ── OOXML boilerplate files ───────────────────────────────── */

    std::string content_types() {
        XmlWriter w;
        w.decl();
        w.open_attr("Types")
         .attr("xmlns", "http://schemas.openxmlformats.org/package/2006/content-types")
         .end_open();
        w.open_attr("Default").attr("Extension", "rels")
         .attr("ContentType", "application/vnd.openxmlformats-package.relationships+xml")
         .self_close();
        w.open_attr("Default").attr("Extension", "xml")
         .attr("ContentType", "application/xml")
         .self_close();
        w.open_attr("Default").attr("Extension", "png")
         .attr("ContentType", "image/png")
         .self_close();
        w.open_attr("Default").attr("Extension", "jpeg")
         .attr("ContentType", "image/jpeg")
         .self_close();
        w.open_attr("Default").attr("Extension", "jpg")
         .attr("ContentType", "image/jpeg")
         .self_close();
        w.open_attr("Override").attr("PartName", "/word/document.xml")
         .attr("ContentType", "application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml")
         .self_close();
        w.open_attr("Override").attr("PartName", "/word/styles.xml")
         .attr("ContentType", "application/vnd.openxmlformats-officedocument.wordprocessingml.styles+xml")
         .self_close();
        if (!num_defs_.empty()) {
            w.open_attr("Override").attr("PartName", "/word/numbering.xml")
             .attr("ContentType", "application/vnd.openxmlformats-officedocument.wordprocessingml.numbering+xml")
             .self_close();
        }
        w.close("Types");
        return w.str();
    }

    std::string top_rels() {
        XmlWriter w;
        w.decl();
        w.open_attr("Relationships")
         .attr("xmlns", "http://schemas.openxmlformats.org/package/2006/relationships")
         .end_open();
        w.open_attr("Relationship").attr("Id", "rId1")
         .attr("Type", "http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument")
         .attr("Target", "word/document.xml").self_close();
        w.close("Relationships");
        return w.str();
    }

    std::string doc_rels() {
        XmlWriter w;
        w.decl();
        w.open_attr("Relationships")
         .attr("xmlns", "http://schemas.openxmlformats.org/package/2006/relationships")
         .end_open();
        w.open_attr("Relationship").attr("Id", "rId1")
         .attr("Type", "http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles")
         .attr("Target", "styles.xml").self_close();
        if (!num_defs_.empty()) {
            w.open_attr("Relationship").attr("Id", "rId2")
             .attr("Type", "http://schemas.openxmlformats.org/officeDocument/2006/relationships/numbering")
             .attr("Target", "numbering.xml").self_close();
        }
        for (auto &h : hyperlinks_) {
            w.open_attr("Relationship").attr("Id", h.first)
             .attr("Type", "http://schemas.openxmlformats.org/officeDocument/2006/relationships/hyperlink")
             .attr("Target", h.second)
             .attr("TargetMode", "External").self_close();
        }
        for (auto &img : images_) {
            w.open_attr("Relationship").attr("Id", img.first)
             .attr("Type", "http://schemas.openxmlformats.org/officeDocument/2006/relationships/image")
             .attr("Target", img.second.substr(5)).self_close(); /* strip "word/" */
        }
        w.close("Relationships");
        return w.str();
    }

    std::string document_xml() {
        XmlWriter w;
        w.decl();
        w.open_attr("w:document")
         .attr("xmlns:w", "http://schemas.openxmlformats.org/wordprocessingml/2006/main")
         .attr("xmlns:r", "http://schemas.openxmlformats.org/officeDocument/2006/relationships")
         .attr("xmlns:wp", "http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing")
         .attr("xmlns:a", "http://schemas.openxmlformats.org/drawingml/2006/main")
         .attr("xmlns:pic", "http://schemas.openxmlformats.org/drawingml/2006/picture")
         .end_open();
        w.open("w:body");
        w.raw(body_.str());
        w.close("w:body");
        w.close("w:document");
        return w.str();
    }

    std::string styles_xml() {
        XmlWriter w;
        w.decl();
        w.open_attr("w:styles")
         .attr("xmlns:w", "http://schemas.openxmlformats.org/wordprocessingml/2006/main")
         .end_open();

        /* Default style */
        w.open_attr("w:docDefaults").end_open();
        w.open_attr("w:rPrDefault").end_open();
        w.open("w:rPr");
        w.open_attr("w:rFonts").attr("w:ascii", "Calibri").attr("w:hAnsi", "Calibri").self_close();
        w.open_attr("w:sz").attr("w:val", 22).self_close();
        w.close("w:rPr");
        w.close("w:rPrDefault");
        w.close("w:docDefaults");

        /* Heading styles 1-6 */
        const int heading_sizes[] = {0, 48, 36, 28, 24, 22, 20};
        for (int i = 1; i <= 6; i++) {
            std::string name = "Heading" + std::to_string(i);
            w.open_attr("w:style").attr("w:type", "paragraph").attr("w:styleId", name).end_open();
            w.open_attr("w:name").attr("w:val", std::string("heading ") + std::to_string(i)).self_close();
            w.open("w:pPr");
            w.open_attr("w:spacing").attr("w:before", 240).attr("w:after", 120).self_close();
            w.close("w:pPr");
            w.open("w:rPr");
            w.raw("<w:b/>");
            w.open_attr("w:sz").attr("w:val", heading_sizes[i]).self_close();
            w.close("w:rPr");
            w.close("w:style");
        }

        w.close("w:styles");
        return w.str();
    }

    std::string numbering_xml() {
        XmlWriter w;
        w.decl();
        w.open_attr("w:numbering")
         .attr("xmlns:w", "http://schemas.openxmlformats.org/wordprocessingml/2006/main")
         .end_open();

        for (auto &nd : num_defs_) {
            std::string aid = std::to_string(nd.num_id);
            w.open_attr("w:abstractNum").attr("w:abstractNumId", aid).end_open();
            for (int lvl = 0; lvl < 9; lvl++) {
                w.open_attr("w:lvl").attr("w:ilvl", lvl).end_open();
                w.open_attr("w:start").attr("w:val", 1).self_close();
                if (nd.ordered) {
                    w.open_attr("w:numFmt").attr("w:val", "decimal").self_close();
                    w.open_attr("w:lvlText").attr("w:val", "%" + std::to_string(lvl+1) + ".").self_close();
                } else {
                    w.open_attr("w:numFmt").attr("w:val", "bullet").self_close();
                    std::string bullet = (lvl == 0) ? "\xE2\x80\xA2" : "\xE2\x97\xA6"; /* • or ◦ */
                    w.open_attr("w:lvlText").attr("w:val", bullet).self_close();
                }
                w.close("w:lvl");
            }
            w.close("w:abstractNum");

            w.open_attr("w:num").attr("w:numId", aid).end_open();
            w.open_attr("w:abstractNumId").attr("w:val", aid).self_close();
            w.close("w:num");
        }

        w.close("w:numbering");
        return w.str();
    }
};

/* ══════════════════════════════════════════════════════════════════
 *  Ruby binding: render_docx(node) → String
 * ══════════════════════════════════════════════════════════════════ */

static VALUE native_render_docx(VALUE mod, VALUE node) {
    DocxRenderer renderer;
    std::string zip_data = renderer.render(node);
    return rb_enc_str_new(zip_data.data(), static_cast<long>(zip_data.size()),
                          rb_ascii8bit_encoding());
}

/* ══════════════════════════════════════════════════════════════════
 *  Init (called from Init_fwui_native)
 * ══════════════════════════════════════════════════════════════════ */

extern "C" void Init_fwui_docx(void) {
    crc32_init();

    VALUE mFWUI = rb_const_get(rb_cObject, rb_intern("FWUI"));
    VALUE mDocx = rb_define_module_under(mFWUI, "NativeDocx");

    rb_define_module_function(mDocx, "render_docx",
        reinterpret_cast<VALUE (*)(...)>(native_render_docx), 1);
}
