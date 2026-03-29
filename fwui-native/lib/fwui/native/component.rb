# frozen_string_literal: true

module FWUI
  module UI
    class Component
      def self.bake(name, &block)
        children = block.call
        children = Array(children)
        template_node = FWUI.div(children)
        FWUI::Native.bake(name, template_node)
      end

      def self.render(name, **params)
        FWUI::Native.render_baked(name, **params)
      end

      def self.node(name, **params)
        FWUI::Native.baked_node(name, **params)
      end
    end
  end
end
