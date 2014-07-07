using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Threading;
using System.Timers;
using System.Windows.Forms;

namespace Zoomer
{
    static class Program
    {
        private static System.Timers.Timer PTimerGC = new System.Timers.Timer(5 * 60 * 1000);
        private static System.Windows.Forms.NotifyIcon PNotifyIcon = new System.Windows.Forms.NotifyIcon();
        private static Mutex mutex = new Mutex(true, "{86199e51-7152-4c54-b7a5-4d9c325e1343}");

        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        [STAThread]
        static void Main()
        {
            if (mutex.WaitOne(TimeSpan.Zero, true))
            {
                PTimerGC.AutoReset = true;
                PTimerGC.Elapsed += PTimerGC_Elapsed;
                PTimerGC.Start();

                Zoomer.start();

                ShowNotifyIcon();

                Application.Run();
            }
        }

        private static void ShowNotifyIcon()
        {
            PNotifyIcon.Icon = System.Drawing.Icon.ExtractAssociatedIcon(Application.ExecutablePath);
            PNotifyIcon.Text = Application.ProductName;
            PNotifyIcon.ContextMenu = new ContextMenu(new MenuItem[] { 
                new MenuItem("Information", MenuInfomation_Click),
                new MenuItem("Exit", MenuExit_Click)
            });
            PNotifyIcon.Visible = true;
        }

        private static void MenuExit_Click(object sender, EventArgs e)
        {
            exit();
        }

        private static void MenuInfomation_Click(object sender, EventArgs e)
        {
            Process.Start("http://nchoang.github.io/zoomer");
        }

        static void PTimerGC_Elapsed(object sender, ElapsedEventArgs e)
        {
            System.GC.Collect();
        }

        static void exit()
        {
            Zoomer.stop();
            PTimerGC.Stop();
            PNotifyIcon.Visible = false;
            System.GC.Collect();
            Application.Exit();
        }
    }
}
