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
using SkiaSharp;

namespace xenly_2d_graphics
{
    public class Renderer : IDisposable
    {
        public int Width { get; }
        public int Height { get; }
        private SKBitmap bitmap;
        private SKCanvas canvas;

        public Renderer(int width, int height)
        {
            Width = width;
            Height = height;
            bitmap = new SKBitmap(width, height);
            canvas = new SKCanvas(bitmap);
        }

        public void draw_circle(int x, int y, int radius, SKColor color)
        {
            using (var paint = new SKPaint { Color = color, IsAntialias = true })
            {
                canvas.DrawCircle(x, y, radius, paint);
            }
        }

        public void Clear(SKColor color)
        {
            canvas.Clear(color);
        }

        public byte[] GetBuffer()
        {
            using (var image = SKImage.FromBitmap(bitmap))
            using (var data = image.Encode(SKEncodedImageFormat.Png, 100))
            {
                return data.ToArray();
            }
        }

        public void save_to_file(string filePath)
        {
            using (var image = SKImage.FromBitmap(bitmap))
            using (var data = image.Encode(SKEncodedImageFormat.Png, 100))
            {
                System.IO.File.WriteAllBytes(filePath, data.ToArray());
            }
        }

        public void Dispose()
        {
            bitmap.Dispose();
            canvas.Dispose();
        }
    }
}
