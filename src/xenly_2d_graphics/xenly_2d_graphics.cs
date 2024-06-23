/*
 * XENLY - high-level and general-purpose programming language
 * created, designed, and developed by Cyril John Magayaga (cjmagayaga957@gmail.com, cyrilmagayaga@proton.me).
 *
 * It is initially written in Rust programming language.
 *
 * It is available for Linux and Windows operating systems.
 *
 */
using System;

namespace xenly_2d_graphics
{
    public class Renderer
    {
        public int Width { get; private set; }
        public int Height { get; private set; }
        private uint[] buffer;

        public Renderer(int width, int height)
        {
            Width = width;
            Height = height;
            buffer = new uint[width * height];
        }

        public void DrawCircle(int x, int y, int radius, uint color)
        {
            for (int i = 0; i < Width; i++)
            {
                for (int j = 0; j < Height; j++)
                {
                    int dx = x - i;
                    int dy = y - j;
                    if (dx * dx + dy * dy <= radius * radius)
                    {
                        buffer[j * Width + i] = color;
                    }
                }
            }
        }

        public uint[] GetBuffer()
        {
            return buffer;
        }
    }
}

// Window class (mocked for demonstration)
public class Window
{
    private bool isOpen;
    private bool[] keysDown;

    public Window(string title, int width, int height)
    {
        isOpen = true;
        keysDown = new bool[256]; // Assuming 256 possible keys
    }

    public bool IsOpen()
    {
        return isOpen;
    }

    public bool IsKeyDown(int key)
    {
        if (key < 0 || key >= keysDown.Length)
            return false;

        return keysDown[key];
    }

    public void Update(uint[] buffer, int width, int height)
    {
        // Perform window update with buffer
        Console.WriteLine("Updating window with buffer...");
    }
}
