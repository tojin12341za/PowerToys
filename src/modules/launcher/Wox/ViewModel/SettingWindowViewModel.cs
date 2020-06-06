using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Net;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using Wox.Core;
using Wox.Core.Plugin;
using Wox.Core.Resource;
using Wox.Helper;
using Wox.Infrastructure;
using Wox.Infrastructure.Http;
using Wox.Infrastructure.Storage;
using Wox.Infrastructure.UserSettings;
using Wox.Plugin;

namespace Wox.ViewModel
{
    public class SettingWindowViewModel : BaseModel
    {
        private readonly WoxJsonStorage<Settings> _storage;

        public SettingWindowViewModel()
        {
            _storage = new WoxJsonStorage<Settings>();
            Settings = _storage.Load();
            Settings.PropertyChanged += (s, e) =>
            {
                if (e.PropertyName == nameof(Settings.ActivateTimes))
                {
                    OnPropertyChanged(nameof(ActivatedTimes));
                }
            };


        }

        public Settings Settings { get; set; }

        public void Save()
        {
            _storage.Save();
        }

        #region general

        // todo a better name?
        public class LastQueryMode
        {
            public string Display { get; set; }
            public Infrastructure.UserSettings.LastQueryMode Value { get; set; }
        }
        public List<LastQueryMode> LastQueryModes
        {
            get
            {
                List<LastQueryMode> modes = new List<LastQueryMode>();
                var enums = (Infrastructure.UserSettings.LastQueryMode[])Enum.GetValues(typeof(Infrastructure.UserSettings.LastQueryMode));
                foreach (var e in enums)
                {
                    var key = $"LastQuery{e}";
                    var display = _translater.GetTranslation(key);
                    var m = new LastQueryMode { Display = display, Value = e, };
                    modes.Add(m);
                }
                return modes;
            }
        }

        public string Language
        {
            get
            {
                return Settings.Language;
            }
            set
            {
                InternationalizationManager.Instance.ChangeLanguage(value);

                if (InternationalizationManager.Instance.PromptShouldUsePinyin(value))
                    ShouldUsePinyin = true;
            }
        }

        public bool ShouldUsePinyin
        {
            get 
            {
                return Settings.ShouldUsePinyin;            
            }
            set 
            {
                Settings.ShouldUsePinyin = value;
            }
        }

        public List<string> QuerySearchPrecisionStrings
        {
            get
            {
                var precisionStrings = new List<string>();

                var enumList = Enum.GetValues(typeof(StringMatcher.SearchPrecisionScore)).Cast<StringMatcher.SearchPrecisionScore>().ToList();

                enumList.ForEach(x => precisionStrings.Add(x.ToString()));

                return precisionStrings;
            }
        }

        private Internationalization _translater => InternationalizationManager.Instance;
        public List<Language> Languages => _translater.LoadAvailableLanguages();
        public IEnumerable<int> MaxResultsRange => Enumerable.Range(2, 16);

        #endregion

        #region plugin

        public static string Plugin => "http://www.wox.one/plugin";
        public PluginViewModel SelectedPlugin { get; set; }

        public IList<PluginViewModel> PluginViewModels
        {
            get
            {
                var metadatas = PluginManager.AllPlugins
                    .OrderBy(x => x.Metadata.Disabled)
                    .ThenBy(y => y.Metadata.Name)
                    .Select(p => new PluginViewModel { PluginPair = p})
                    .ToList();
                return metadatas;
            }
        }

        public Control SettingProvider
        {
            get
            {
                var settingProvider = SelectedPlugin.PluginPair.Plugin as ISettingProvider;
                if (settingProvider != null)
                {
                    var control = settingProvider.CreateSettingPanel();
                    control.HorizontalAlignment = HorizontalAlignment.Stretch;
                    control.VerticalAlignment = VerticalAlignment.Stretch;
                    return control;
                }
                else
                {
                    return new Control();
                }
            }
        }



        #endregion

        #region theme

        public static string Theme => @"http://www.wox.one/theme/builder";

        public string SelectedTheme
        {
            get { return Settings.Theme; }
            set
            {
                Settings.Theme = value;
            }
        }

        public Brush PreviewBackground
        {
            get
            {
                var wallpaper = WallpaperPathRetrieval.GetWallpaperPath();
                if (wallpaper != null && File.Exists(wallpaper))
                {
                    var memStream = new MemoryStream(File.ReadAllBytes(wallpaper));
                    var bitmap = new BitmapImage();
                    bitmap.BeginInit();
                    bitmap.StreamSource = memStream;
                    bitmap.EndInit();
                    var brush = new ImageBrush(bitmap) { Stretch = Stretch.UniformToFill };
                    return brush;
                }
                else
                {
                    var wallpaperColor = WallpaperPathRetrieval.GetWallpaperColor();
                    var brush = new SolidColorBrush(wallpaperColor);
                    return brush;
                }
            }
        }

        public ResultsViewModel PreviewResults
        {
            get
            {
                var results = new List<Result>
                {
                    new Result
                    {
                        Title = "WoX is a launcher for Windows that simply works.",
                        SubTitle = "You can call it Windows omni-eXecutor if you want a long name."
                    },
                    new Result
                    {
                        Title = "Search for everything—applications, folders, files and more.",
                        SubTitle = "Use pinyin to search for programs. (yyy / wangyiyun → 网易云音乐)"
                    },
                    new Result
                    {
                        Title = "Keyword plugin search.",
                        SubTitle = "search google with g search_term."
                    },
                    new Result
                    {
                        Title = "Build custom themes at: ",
                        SubTitle = Theme
                    },
                    new Result
                    {
                        Title = "Install plugins from: ",
                        SubTitle = Plugin
                    },
                };
                var vm = new ResultsViewModel();
                vm.AddResults(results, "PREVIEW");
                return vm;
            }
        }

        public FontFamily SelectedQueryBoxFont
        {
            get
            {
                if (Fonts.SystemFontFamilies.Count(o =>
                    o.FamilyNames.Values != null &&
                    o.FamilyNames.Values.Contains(Settings.QueryBoxFont)) > 0)
                {
                    var font = new FontFamily(Settings.QueryBoxFont);
                    return font;
                }
                else
                {
                    var font = new FontFamily("Segoe UI");
                    return font;
                }
            }
            set
            {
                Settings.QueryBoxFont = value.ToString();
            }
        }

        public FamilyTypeface SelectedQueryBoxFontFaces
        {
            get
            {
                var typeface = SyntaxSugars.CallOrRescueDefault(
                    () => SelectedQueryBoxFont.ConvertFromInvariantStringsOrNormal(
                        Settings.QueryBoxFontStyle,
                        Settings.QueryBoxFontWeight,
                        Settings.QueryBoxFontStretch
                        ));
                return typeface;
            }
            set
            {
                Settings.QueryBoxFontStretch = value.Stretch.ToString();
                Settings.QueryBoxFontWeight = value.Weight.ToString();
                Settings.QueryBoxFontStyle = value.Style.ToString();
            }
        }

        public FontFamily SelectedResultFont
        {
            get
            {
                if (Fonts.SystemFontFamilies.Count(o =>
                    o.FamilyNames.Values != null &&
                    o.FamilyNames.Values.Contains(Settings.ResultFont)) > 0)
                {
                    var font = new FontFamily(Settings.ResultFont);
                    return font;
                }
                else
                {
                    var font = new FontFamily("Segoe UI");
                    return font;
                }
            }
            set
            {
                Settings.ResultFont = value.ToString();
            }
        }

        public FamilyTypeface SelectedResultFontFaces
        {
            get
            {
                var typeface = SyntaxSugars.CallOrRescueDefault(
                    () => SelectedResultFont.ConvertFromInvariantStringsOrNormal(
                        Settings.ResultFontStyle,
                        Settings.ResultFontWeight,
                        Settings.ResultFontStretch
                        ));
                return typeface;
            }
            set
            {
                Settings.ResultFontStretch = value.Stretch.ToString();
                Settings.ResultFontWeight = value.Weight.ToString();
                Settings.ResultFontStyle = value.Style.ToString();
            }
        }

        #endregion

        #region hotkey

        public CustomPluginHotkey SelectedCustomPluginHotkey { get; set; }

        #endregion

        #region about
        public static string Version => Constant.Version;
        public string ActivatedTimes => string.Format(_translater.GetTranslation("about_activate_times"), Settings.ActivateTimes);
        #endregion
    }
}
