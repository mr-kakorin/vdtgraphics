/// Copyright (c) Vito Domenico Tagliente

#pragma once

#include "image.h"

namespace graphics
{
	class Texture
	{
	public:

		typedef unsigned int id_t;

		struct Options
		{
			Options();

			// Wrapping mode on S axis
			unsigned int wrapS;
			// Wrapping mode on T axis 
			unsigned int wrapT;
			// Filtering mode if texture pixels < screen pixels
			unsigned int filterMin;
			// Filtering mode if texture pixels > screen pixels
			unsigned int filterMax;
		};

		Texture(const Image& image, const Options& options = Options{});
		Texture(const Texture&) = delete;
		virtual ~Texture();

		inline id_t getId() const { return m_id; }
		inline bool isValid() const { return m_id != INVALID_ID; }
		inline operator bool() const { return m_id != INVALID_ID; }

		inline unsigned int getWidth() const { return m_width; }
		inline unsigned int getHeight() const { return m_height; }

		virtual void bind() = 0;
		virtual void bind(const unsigned int slot) = 0;
		virtual void unbind() = 0;

		Texture& operator=(const Texture& texture) = delete;

		bool operator==(const Texture& texture) const
		{
			return m_id == texture.getId();
		}

		bool operator!=(const Texture& texture) const
		{
			return m_id != texture.getId();
		}

		static constexpr id_t INVALID_ID = 0;

	protected:

		// texture id
		id_t m_id;
		// texture size
		unsigned int m_width, m_height;
	};
}