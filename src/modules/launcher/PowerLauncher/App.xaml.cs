using Microsoft.PowerLauncher.Telemetry;
using Microsoft.PowerToys.Telemetry;
using System;
using System.Diagnostics;
using System.Threading.Tasks;
using System.Timers;
using System.Windows;
using Wox;
using Wox.Core;
using Wox.Core.Plugin;
using Wox.Core.Resource;
using Wox.Helper;
using Wox.Infrastructure;
using Wox.Infrastructure.Http;
using Wox.Infrastructure.Image;
using Wox.Infrastructure.Logger;
using Wox.Infrastructure.UserSettings;
using Wox.ViewModel;
using Stopwatch = Wox.Infrastructure.Stopwatch;

namespace PowerLauncher
{
    

    public partial class App : IDisposable, ISingleInstanceApp
    {
        public static PublicAPIInstance API { get; private set; }
        private const string Unique = "PowerLauncher_Unique_Application_Mutex";
        private static bool _disposed;
        private Settings _settings;
        private MainViewModel _mainVM;
        private SettingWindowViewModel _settingsVM;
        private readonly Alphabet _alphabet = new Alphabet();
        private StringMatcher _stringMatcher;
        private SettingsWatcher _settingsWatcher;

        [STAThread]
        public static void Main()
        {
            if (SingleInstance<App>.InitializeAsFirstInstance(Unique))
            {
                using (var application = new App())
                {
                    application.InitializeComponent();
                    application.Run();
                }
            }
        }

        private void OnStartup(object sender, StartupEventArgs e)
        {
            var bootTime = new System.Diagnostics.Stopwatch();
            bootTime.Start();
            Stopwatch.Normal("|App.OnStartup|Startup cost", () =>
            {
                Log.Info("|App.OnStartup|Begin Wox startup ----------------------------------------------------");
                Log.Info($"|App.OnStartup|Runtime info:{ErrorReporting.RuntimeInfo()}");
                RegisterAppDomainExceptions();
                RegisterDispatcherUnhandledException();

                ImageLoader.Initialize();

                _settingsVM = new SettingWindowViewModel();
                _settings = _settingsVM.Settings;

                _alphabet.Initialize(_settings);
                _stringMatcher = new StringMatcher(_alphabet);
                StringMatcher.Instance = _stringMatcher;
                _stringMatcher.UserSettingSearchPrecision = _settings.QuerySearchPrecision;

                ThemeManager themeManager = new ThemeManager(this);               
                PluginManager.LoadPlugins(_settings.PluginSettings);
                _mainVM = new MainViewModel(_settings);
                var window = new MainWindow(_settings, _mainVM);
                API = new PublicAPIInstance(_settingsVM, _mainVM, _alphabet);
                PluginManager.InitializePlugins(API);

                Current.MainWindow = window;
                Current.MainWindow.Title = Constant.ExeFileName;

                // happlebao todo temp fix for instance code logic
                // load plugin before change language, because plugin language also needs be changed
                InternationalizationManager.Instance.Settings = _settings;
                InternationalizationManager.Instance.ChangeLanguage(_settings.Language);

                // main windows needs initialized before theme change because of blur settings

                Http.Proxy = _settings.Proxy;

                RegisterExitEvents();

                _settingsWatcher = new SettingsWatcher(_settings);
                
                _mainVM.MainWindowVisibility = Visibility.Visible;
                _mainVM.ColdStartFix();
                Log.Info("|App.OnStartup|End Wox startup ----------------------------------------------------  ");

                bootTime.Stop();

                PowerToysTelemetry.Log.WriteEvent(new LauncherBootEvent() { BootTimeMs = bootTime.ElapsedMilliseconds });

                //[Conditional("RELEASE")]
                // check update every 5 hours

                // check updates on startup
            });
        }

        private void RegisterExitEvents()
        {
            AppDomain.CurrentDomain.ProcessExit += (s, e) => Dispose();
            Current.Exit += (s, e) => Dispose();
            Current.SessionEnding += (s, e) => Dispose();
        }

        /// <summary>
        /// let exception throw as normal is better for Debug
        /// </summary>
        [Conditional("RELEASE")]
        private void RegisterDispatcherUnhandledException()
        {
            DispatcherUnhandledException += ErrorReporting.DispatcherUnhandledException;
        }


        /// <summary>
        /// let exception throw as normal is better for Debug
        /// </summary>
        [Conditional("RELEASE")]
        private static void RegisterAppDomainExceptions()
        {
            AppDomain.CurrentDomain.UnhandledException += ErrorReporting.UnhandledExceptionHandle;
        }

        public void Dispose()
        {
            // if sessionending is called, exit proverbially be called when log off / shutdown
            // but if sessionending is not called, exit won't be called when log off / shutdown
            if (!_disposed)
            {
                API.SaveAppAllSettings();
                _disposed = true;
            }
        }

        public void OnSecondAppStarted()
        {
            Current.MainWindow.Visibility = Visibility.Visible;
        }
    }
}