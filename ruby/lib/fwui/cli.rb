# frozen_string_literal: true

require 'fileutils'

module FWUI
  module CLI
    module_function

    def run(args)
      command = args.shift

      case command
      when 'build'   then build(args)
      when 'serve'   then serve(args)
      when 'new'     then new_resource(args)
      when 'version', '-v', '--version'
        puts "fwui #{FWUI::VERSION}"
      else
        help
      end
    end

    # ── build ────────────────────────────────────────────────────

    def build(args)
      project_root = Dir.pwd

      pages_dir = File.join(project_root, 'ruby', 'pages')
      pages_dir = File.join(project_root, 'pages') unless Dir.exist?(pages_dir)

      static_dir = File.join(project_root, 'static')
      dist_dir = args.shift || File.join(project_root, 'dist')

      unless Dir.exist?(pages_dir)
        abort "Error: pages directory not found (tried pages/ and ruby/pages/)"
      end

      # Load pages
      registry = FWUI::Registry.new
      loaded = FWUI::PageLoader.load_pages(registry, pages_dir)

      if loaded.empty?
        abort "Error: no pages found in #{pages_dir}"
      end

      # Prepare dist
      FileUtils.rm_rf(dist_dir)
      FileUtils.mkdir_p(dist_dir)

      # Render each page
      rendered = 0
      registry.page_routes.each do |route|
        page = registry.create_page(route)
        html = page.to_html

        # Route → filename: "/" → "index.html", "/projects" → "projects/index.html"
        if route == '/'
          file_path = File.join(dist_dir, 'index.html')
        else
          clean = route.sub(%r{^/}, '').sub(%r{/$}, '')
          file_path = File.join(dist_dir, clean, 'index.html')
        end

        FileUtils.mkdir_p(File.dirname(file_path))
        File.write(file_path, html)
        rendered += 1
        puts "  #{route} → #{file_path.sub(project_root + '/', '')}"
      end

      # Copy static files
      if Dir.exist?(static_dir)
        static_dest = File.join(dist_dir, 'static')
        FileUtils.cp_r(static_dir, static_dest)
        static_count = Dir.glob(File.join(static_dir, '**', '*')).count { |f| File.file?(f) }
        puts "  static/ → #{static_count} files copied"
      end

      puts "\nBuild complete: #{rendered} pages → #{dist_dir}/"
    end

    # ── serve ────────────────────────────────────────────────────

    def serve(args)
      server_path = find_server
      abort "Error: server.rb not found" unless server_path

      # Default to dev mode
      args << '--dev' unless args.include?('--dev') || args.include?('--prod')
      args.delete('--prod')

      exec('ruby', server_path, *args)
    end

    # ── new ──────────────────────────────────────────────────────

    def new_resource(args)
      sub = args.first
      case sub
      when 'page'
        args.shift
        new_page(args)
      else
        new_project(args)
      end
    end

    def new_page(args)
      name = args.shift
      abort "Usage: fwui new page <name>" unless name

      pages_dir = File.join(Dir.pwd, 'pages')
      pages_dir = File.join(Dir.pwd, 'ruby', 'pages') unless Dir.exist?(pages_dir)
      FileUtils.mkdir_p(pages_dir) unless Dir.exist?(pages_dir)

      snake = name.gsub(/([A-Z])/, '_\1').sub(/^_/, '').downcase.gsub(/[^a-z0-9_]/, '_')
      camel = snake.split('_').map(&:capitalize).join
      route = "/#{snake.tr('_', '-')}"
      file_path = File.join(pages_dir, "#{snake}.rb")

      if File.exist?(file_path)
        abort "Error: #{file_path} already exists"
      end

      File.write(file_path, <<~RUBY)
        # frozen_string_literal: true

        module Pages
          module #{camel}
            extend FWUI::DSL
            module_function

            def register(registry)
              registry.register_page("#{route}") do |data|
                layout("#{camel}",
                  head: [stylesheet("/static/style.css")],
                  navbar: [
                    a("Home", href: "/") | AddClass("nav-link"),
                    a("#{camel}", href: "#{route}") | AddClass("nav-link active"),
                  ],
                  body: [
                    h1("#{camel}") | Bold(),
                    p("Edit this page in pages/#{snake}.rb"),
                  ],
                  footer: [p("Built with FWUI") | Center()]
                )
              end
            end
          end
        end
      RUBY

      puts "  #{file_path}"
      puts "  route: #{route}"
    end

    def new_project(args)
      name = args.shift
      abort "Usage: fwui new <project-name>" unless name

      if Dir.exist?(name)
        abort "Error: directory '#{name}' already exists"
      end

      puts "Creating #{name}/"

      FileUtils.mkdir_p(File.join(name, 'pages'))
      FileUtils.mkdir_p(File.join(name, 'static'))
      FileUtils.mkdir_p(File.join(name, 'content'))

      # Sample page
      File.write(File.join(name, 'pages', 'home.rb'), <<~RUBY)
        # frozen_string_literal: true

        module Pages
          module Home
            extend FWUI::DSL
            module_function

            def register(registry)
              registry.register_page("/") do |_data|
                layout("#{name}",
                  head: [stylesheet("/static/style.css")],
                  navbar: [
                    a("Home", href: "/") | AddClass("nav-link"),
                  ],
                  body: [
                    h1("#{name}") | Bold() | Center(),
                    p("Built with FWUI") | Center(),
                  ],
                  footer: [p("Built with FWUI") | Center()]
                )
              end
            end
          end
        end
      RUBY

      # Minimal CSS
      File.write(File.join(name, 'static', 'style.css'), <<~CSS)
        * { margin: 0; padding: 0; box-sizing: border-box; }
        body { font-family: system-ui, sans-serif; line-height: 1.6; color: #333; }
        .navbar { padding: 16px 24px; border-bottom: 1px solid #eee; }
        .nav-link { color: inherit; text-decoration: none; }
        .container { max-width: 800px; margin: 0 auto; padding: 40px 24px; }
      CSS

      puts "  pages/home.rb"
      puts "  static/style.css"
      puts "  content/"
      puts "\nDone. Next:"
      puts "  cd #{name}"
      puts "  fwui build     # generate dist/"
      puts "  fwui serve     # start dev server"
    end

    # ── help ─────────────────────────────────────────────────────

    def help
      puts <<~HELP
        fwui #{FWUI::VERSION} — declarative HTML generation

        Commands:
          build [dist_dir]    Render pages/ to dist/ (default)
          serve [--dev]       Start development server with hot-reload
          new <name>          Create new project scaffold
          new page <name>     Generate page with layout boilerplate
          version             Show version

        Project structure:
          pages/              Page definitions (*.rb)
          static/             Static assets (CSS, JS, images)
          content/            Markdown content files
          dist/               Build output (generated)
      HELP
    end

    # ── helpers ──────────────────────────────────────────────────

    def find_server
      candidates = [
        File.join(Dir.pwd, 'ruby', 'examples', 'server.rb'),
        File.join(Dir.pwd, 'server.rb'),
        File.expand_path('../../examples/server.rb', __dir__),
      ]
      candidates.find { |p| File.exist?(p) }
    end

  end
end