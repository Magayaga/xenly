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
using System.Threading;

namespace xenly_2d_graphics
{
    public class Window
    {
        private Renderer renderer;
        private int width;
        private int height;
        private bool isOpen;

        public Window(int width, int height)
        {
            this.width = width;
            this.height = height;
            this.renderer = new Renderer(width, height); // Initialize with a new Renderer
            this.isOpen = true;
        }

        public void Show(Renderer renderer)
        {
            this.renderer = renderer;

            // Start a separate thread to simulate window display
            System.Threading.Thread displayThread = new System.Threading.Thread(DisplayLoop);
            displayThread.Start();
        }

        private void DisplayLoop()
        {
            int frame = 0;
            while (isOpen)
            {
                string fileName = $"frame_{frame++:D4}.png";
                renderer.SaveToFile(fileName);
                System.Console.WriteLine($"Saved {fileName}");
                System.Threading.Thread.Sleep(1000 / 30); // Simulate 30 FPS
            }
        }

        public void Update()
        {
            // In this example, Update does nothing specific. It's here for demonstration.
            System.Console.WriteLine("Updating window content...");
        }

        public void Close()
        {
            isOpen = false;
            System.Console.WriteLine("Window closed.");
        }
    }
}
