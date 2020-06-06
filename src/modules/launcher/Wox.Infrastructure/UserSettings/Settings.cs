using System;
using System.Collections.ObjectModel;
using System.Drawing;
using Newtonsoft.Json;
using Newtonsoft.Json.Converters;
using Wox.Plugin;

namespace Wox.Infrastructure.UserSettings
{
    public class Settings : BaseModel
    {
        private string _hotkey = "Alt + Space";
        private string _previousHotkey = "";
        public string PreviousHotkey {
            get
            {
                return _previousHotkey;
            }
        }
        public string Hotkey 
        {
            get
            {
                return _hotkey;
            }
            set
            {
                if (_hotkey != value)
                {
                    _previousHotkey = _hotkey;
                    _hotkey = value;
                    OnPropertyChanged();
                }
            }
        }
        public string Language { get; set; } = "en";
        public string Theme { get; set; } = "Dark";
        public string QueryBoxFont { get; set; } = FontFamily.GenericSansSerif.Name;
        public string QueryBoxFontStyle { get; set; }
        public string QueryBoxFontWeight { get; set; }
        public string QueryBoxFontStretch { get; set; }
        public string ResultFont { get; set; } = FontFamily.GenericSansSerif.Name;
        public string ResultFontStyle { get; set; }
        public string ResultFontWeight { get; set; }
        public string ResultFontStretch { get; set; }


        /// <summary>
        /// when false Alphabet static service will always return empty results
        /// </summary>
        public bool ShouldUsePinyin { get; set; } = false;

        internal StringMatcher.SearchPrecisionScore QuerySearchPrecision { get; private set; } = StringMatcher.SearchPrecisionScore.Regular;

        [JsonIgnore]
        public string QuerySearchPrecisionString
        {
            get { return QuerySearchPrecision.ToString(); }
            set
            {
                try
                {
                    var precisionScore = (StringMatcher.SearchPrecisionScore)Enum
                                            .Parse(typeof(StringMatcher.SearchPrecisionScore), value);

                    QuerySearchPrecision = precisionScore;
                    StringMatcher.Instance.UserSettingSearchPrecision = precisionScore;
                }
                catch (ArgumentException e)
                {
                    Logger.Log.Exception(nameof(Settings), "Failed to load QuerySearchPrecisionString value from Settings file", e);

                    QuerySearchPrecision = StringMatcher.SearchPrecisionScore.Regular;
                    StringMatcher.Instance.UserSettingSearchPrecision = StringMatcher.SearchPrecisionScore.Regular;

                    throw;
                }
            }
        }

        public bool AutoUpdates { get; set; } = false;

        public double WindowLeft { get; set; }
        public double WindowTop { get; set; }

        private int _maxResultsToShow = 4;
        public int MaxResultsToShow 
        {
            get
            {
                return _maxResultsToShow;
            }
            set
            {
                if (_maxResultsToShow != value)
                {
                    _maxResultsToShow = value;
                    OnPropertyChanged();
                }
            }
        }
        public int ActivateTimes { get; set; }

        // Order defaults to 0 or -1, so 1 will let this property appear last
        [JsonProperty(Order = 1)]
        public PluginsSettings PluginSettings { get; set; } = new PluginsSettings();
        public ObservableCollection<CustomPluginHotkey> CustomPluginHotkeys { get; set; } = new ObservableCollection<CustomPluginHotkey>();

        [Obsolete]
        public double Opacity { get; set; } = 1;

        [Obsolete]
        public OpacityMode OpacityMode { get; set; } = OpacityMode.Normal;

        public bool DontPromptUpdateMsg { get; set; }
        public bool EnableUpdateLog { get; set; }

        public bool StartWoxOnSystemStartup { get; set; } = true;
        public bool HideOnStartup { get; set; }
        bool _hideNotifyIcon { get; set; }
        public bool HideNotifyIcon
        {
            get { return _hideNotifyIcon; }
            set
            {
                _hideNotifyIcon = value;
                OnPropertyChanged();
            }
        }
        public bool LeaveCmdOpen { get; set; }
        public bool HideWhenDeactivated { get; set; } = true;
        public bool RememberLastLaunchLocation { get; set; }
        public bool IgnoreHotkeysOnFullscreen { get; set; }

        public HttpProxy Proxy { get; set; } = new HttpProxy();

        [JsonConverter(typeof(StringEnumConverter))]
        public LastQueryMode LastQueryMode { get; set; } = LastQueryMode.Selected;
    }

    public enum LastQueryMode
    {
        Selected,
        Empty,
        Preserved
    }

    [Obsolete]
    public enum OpacityMode
    {
        Normal = 0,
        LayeredWindow = 1,
        DWM = 2
    }
}