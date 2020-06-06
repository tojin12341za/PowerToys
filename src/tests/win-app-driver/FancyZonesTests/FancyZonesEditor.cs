using Microsoft.VisualStudio.TestTools.UnitTesting;
using OpenQA.Selenium.Appium.Windows;
using OpenQA.Selenium.Interactions;

namespace PowerToysTests
{
    public class FancyZonesEditor : PowerToysSession
    {
        protected static WindowsElement editorWindow;

        protected static void ResetSettings()
        {
            ResetDefaultFancyZonesSettings(false);
            ResetDefaultZoneSettings(true);
        }

        protected static void OpenEditor()
        {
            new Actions(session).KeyDown(OpenQA.Selenium.Keys.Command).SendKeys("`").KeyUp(OpenQA.Selenium.Keys.Command).Perform();
            WaitSeconds(2);
            //editorWindow = WaitElementByXPath("//Window[@Name=\"FancyZones Editor\"]");
            editorWindow = WaitElementByName("FancyZones Editor");
            //may not find editor by name in 0.16.1
            //editorWindow = WaitElementByAccessibilityId("MainWindow1");
            Assert.IsNotNull(editorWindow, "Couldn't find editor window");
        }

        protected static void CloseEditor()
        {
            try
            {
                if (editorWindow != null)
                {
                    editorWindow.SendKeys(OpenQA.Selenium.Keys.Alt + OpenQA.Selenium.Keys.F4);
                }
            }
            catch (OpenQA.Selenium.WebDriverException)
            {
                //editor has been already closed
            }
        }

        protected static void OpenCustomLayouts()
        {
            WindowsElement customsTab = session.FindElementByName("Custom");
            customsTab.Click();
            string isSelected = customsTab.GetAttribute("SelectionItem.IsSelected");
            Assert.AreEqual("True", isSelected, "Custom tab cannot be opened");
        }

        protected static void OpenTemplates()
        {
            WindowsElement templatesTab = session.FindElementByName("Templates");
            templatesTab.Click();
            string isSelected = templatesTab.GetAttribute("SelectionItem.IsSelected");
            Assert.AreEqual("True", isSelected, "Templates tab cannot be opened");
        }

        protected static void OpenCreatorWindow(string tabName, string creatorWindowName, string buttonId = "EditCustomButton")
        {
            string elementXPath = "//Text[@Name=\"" + tabName + "\"]";
            WaitElementByXPath(elementXPath).Click();
            WaitElementByAccessibilityId(buttonId).Click();

            WindowsElement creatorWindow = WaitElementByName(creatorWindowName);
            Assert.IsNotNull(creatorWindow, "Creator window didn't open");
        }

        protected void ZoneCountTest(int canvasZoneCount, int gridZoneCount)
        {
            Assert.AreEqual(canvasZoneCount, session.FindElementsByClassName("CanvasZone").Count);
            Assert.AreEqual(gridZoneCount, session.FindElementsByClassName("GridZone").Count);
        }
    }
}