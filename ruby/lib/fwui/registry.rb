# frozen_string_literal: true

module FWUI
  # Component and page registry — pure Ruby mirror of C++ fwui::Registry.
  #
  # Usage:
  #   reg = FWUI::Registry.new
  #   reg.register_component("header") { |data| FWUI.h1("Hello") }
  #   reg.register_component("navbar", cache: true) { |data| FWUI.nav([...]) }
  #   reg.register_page("/") { |data| FWUI.document("Home", body: [...]) }
  #   reg.create_page("/")
  class Registry
    def initialize
      @components = {}
      @pages = {}
      @component_cache = {}
      @page_cache = {}
    end

    # --- Component registration ---

    def register_component(name, cache: false, &factory)
      raise ArgumentError, "block required" unless block_given?
      key = name.to_s
      @components[key] = { factory: factory, cache: cache }
      @component_cache.delete(key)
      self
    end

    def unregister_component(name)
      key = name.to_s
      @components.delete(key)
      @component_cache.delete(key)
      self
    end

    def has_component?(name)
      @components.key?(name.to_s)
    end

    def create_component(name, data = {})
      key = name.to_s
      entry = @components[key]
      raise KeyError, "Component not found: #{name}" unless entry

      if entry[:cache]
        cache_key = entry[:cache] == true ? key : "#{key}:#{data.hash}"
        cached = @component_cache[cache_key]
        return FWUI.raw(cached) if cached

        node = entry[:factory].call(data)
        html = node.to_html
        @component_cache[cache_key] = html.freeze
        node
      else
        entry[:factory].call(data)
      end
    end

    # --- Page registration ---

    def register_page(route, cache: false, &factory)
      raise ArgumentError, "block required" unless block_given?
      key = route.to_s
      @pages[key] = { factory: factory, cache: cache }
      @page_cache.delete(key)
      self
    end

    def unregister_page(route)
      key = route.to_s
      @pages.delete(key)
      @page_cache.delete(key)
      self
    end

    def has_page?(route)
      @pages.key?(route.to_s)
    end

    def create_page(route, data = {})
      key = route.to_s
      entry = @pages[key]
      raise KeyError, "Page not found: #{route}" unless entry

      if entry[:cache]
        cache_key = entry[:cache] == true ? key : "#{key}:#{data.hash}"
        cached = @page_cache[cache_key]
        return FWUI.raw(cached) if cached

        node = entry[:factory].call(data)
        html = node.to_html
        @page_cache[cache_key] = html.freeze
        node
      else
        entry[:factory].call(data)
      end
    end

    # --- Cache management ---

    def invalidate_cache(name = nil)
      if name
        key = name.to_s
        @component_cache.delete_if { |k, _| k == key || k.start_with?("#{key}:") }
        @page_cache.delete_if { |k, _| k == key || k.start_with?("#{key}:") }
      else
        @component_cache.clear
        @page_cache.clear
      end
      self
    end

    # --- Introspection ---

    def component_names
      @components.keys.sort
    end

    def page_routes
      @pages.keys.sort
    end

    def cache_stats
      {
        components: @component_cache.size,
        pages: @page_cache.size,
        component_keys: @component_cache.keys,
        page_keys: @page_cache.keys,
      }
    end
  end
end
