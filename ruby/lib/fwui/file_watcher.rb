# frozen_string_literal: true

module FWUI
  class FileWatcher
    def initialize(directories, extensions: ['.rb'], interval: 0.5)
      @directories = Array(directories)
      @extensions = extensions
      @interval = interval
      @mtimes = {}
      @running = false
      @on_change = nil

      scan_files.each { |f| @mtimes[f] = File.mtime(f) }
    end

    def on_change(&block)
      @on_change = block
    end

    def start
      @running = true
      @thread = Thread.new do
        while @running
          changed = detect_changes
          @on_change.call(changed) if changed.any? && @on_change
          sleep @interval
        end
      end
      @thread
    end

    def stop
      @running = false
      @thread&.join(2)
    end

    private

    def scan_files
      @directories.flat_map do |dir|
        next [] unless Dir.exist?(dir)
        Dir.glob(File.join(dir, '**', '*')).select do |f|
          File.file?(f) && @extensions.include?(File.extname(f))
        end
      end
    end

    def detect_changes
      changed = []
      current_files = scan_files

      current_files.each do |file|
        mtime = File.mtime(file)
        if @mtimes[file].nil? || @mtimes[file] != mtime
          changed << file
          @mtimes[file] = mtime
        end
      end

      deleted = @mtimes.keys - current_files
      deleted.each { |f| @mtimes.delete(f) }

      changed
    end
  end
end
