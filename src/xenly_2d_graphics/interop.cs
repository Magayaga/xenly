/*
 * XENLY - high-level and general-purpose programming language
 * created, designed, and developed by Cyril John Magayaga (cjmagayaga957@gmail.com, cyrilmagayaga@proton.me).
 *
 * It is initially written in C# programming language.
 *
 * It is available for Linux and Windows operating systems.
 *
 */
using System;
using System.Runtime.InteropServices;
using SkiaSharp;

namespace xenly_2d_graphics
{
    public static class Interop
    {
        [DllImport("xenly_2d_graphics.dll")]
        public static extern IntPtr Renderer_create(int width, int height);

        [DllImport("xenly_2d_graphics.dll")]
        public static extern void Renderer_draw_circle(IntPtr renderer, int x, int y, int radius, uint color);

        [DllImport("xenly_2d_graphics.dll")]
        public static extern IntPtr Renderer_get_buffer(IntPtr renderer);

        [DllImport("xenly_2d_graphics.dll")]
        public static extern void Renderer_free(IntPtr renderer);

        [DllImport("xenly_2d_graphics.dll")]
        public static extern IntPtr Window_create(int width, int height);

        [DllImport("xenly_2d_graphics.dll")]
        public static extern void Window_free(IntPtr window);

        [DllImport("xenly_2d_graphics.dll")]
        public static extern void Window_update(IntPtr window, IntPtr buffer, int width, int height);
    }
}
