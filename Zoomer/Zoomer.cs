using System;
using System.Collections.Generic;
using System.Text;
using System.Diagnostics;
using System.Windows.Forms;
using System.Timers;

using Gma.UserActivityMonitor;
using WindowsInput;

namespace Zoomer
{
    class Zoomer
    {
        #region variable ***********************************************************************

        private static bool ZmStarted = false;
        private static bool ZmMouseRightIsDown = false;
        private static bool ZmVirtualLControlIsDown = false;

        private static bool ZmIgnoreMouseHandle = false;

        private static InputSimulator ZmInSim = new InputSimulator();

        private static System.Timers.Timer ZmTimerSendRightClick = new System.Timers.Timer(1);

        #endregion variable ////////////////////////////////////////////////////////////////////

        #region public api ********************************************************************

        public static void start()
        {
            if (!ZmStarted)
            {
                HookManager.MouseDown += HookManager_MouseDown;
                HookManager.MouseUp += HookManager_MouseUp;
                HookManager.MouseWheel += HookManager_MouseWheel;

                ZmTimerSendRightClick.Elapsed += ZmTimerSendRightClick_Elapsed;

                ZmInSim.Keyboard.KeyDown(WindowsInput.Native.VirtualKeyCode.LCONTROL);
                ZmInSim.Keyboard.KeyUp(WindowsInput.Native.VirtualKeyCode.LCONTROL);
                ZmMouseRightIsDown = false;
                ZmVirtualLControlIsDown = false;

                ZmStarted = true;
#if DEBUG
                Debug.WriteLine("Zoomer started");
#endif
            }
        }

        public static void stop()
        {
            if (ZmStarted)
            {
                HookManager.MouseDown -= HookManager_MouseDown;
                HookManager.MouseUp -= HookManager_MouseUp;
                HookManager.MouseWheel -= HookManager_MouseWheel;
                ZmTimerSendRightClick.Elapsed -= ZmTimerSendRightClick_Elapsed;
                ZmInSim.Keyboard.KeyDown(WindowsInput.Native.VirtualKeyCode.LCONTROL);
                ZmInSim.Keyboard.KeyUp(WindowsInput.Native.VirtualKeyCode.LCONTROL);

                ZmStarted = false;
            }
        }

        #endregion public api ///////////////////////////////////////////////////////////////////

        #region Events *************************************************************************

        private static void ZmTimerSendRightClick_Elapsed(object sender, ElapsedEventArgs e)
        {
            ZmIgnoreMouseHandle = true;
            ZmInSim.Mouse.RightButtonClick();
            ZmIgnoreMouseHandle = false;
            ZmTimerSendRightClick.Stop();
        }

        private static void HookManager_MouseDown(object sender, MouseEventExtArgs e)
        {
            if (ZmIgnoreMouseHandle) return;
            if (e.Button == MouseButtons.Right)
            {
                ZmMouseRightIsDown = true;
                e.Handled = true;
            }
        }

        private static void HookManager_MouseUp(object sender, MouseEventExtArgs e)
        {
            if (ZmIgnoreMouseHandle) return;
            if (e.Button == MouseButtons.Right)
            {
                if (ZmVirtualLControlIsDown)
                {
                    ZmInSim.Keyboard.KeyUp(WindowsInput.Native.VirtualKeyCode.LCONTROL);
                    ZmVirtualLControlIsDown = false;
                }
                else
                {
                    ZmTimerSendRightClick.Start();
                }
                ZmMouseRightIsDown = false;
                e.Handled = true;
            }
        }


        private static void HookManager_MouseWheel(object sender, MouseEventExtArgs e)
        {
            if (ZmIgnoreMouseHandle) return;
            if (ZmMouseRightIsDown)
            {
                if (!ZmVirtualLControlIsDown)
                {
                    ZmInSim.Keyboard.KeyDown(WindowsInput.Native.VirtualKeyCode.LCONTROL);
                    ZmVirtualLControlIsDown = true;
                }
            }
        }

        #endregion Events //////////////////////////////////////////////////////////////////////
    }
}
