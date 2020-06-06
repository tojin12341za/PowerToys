﻿using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using Wox.Infrastructure.UserSettings;
using Wox.Plugin;

namespace Wox.ViewModel
{
    public class ResultsViewModel : BaseModel
    {
        #region Private Fields

        public ResultCollection Results { get; }

        private readonly object _addResultsLock = new object();
        private readonly object _collectionLock = new object();
        private readonly Settings _settings;
        // private int MaxResults => _settings?.MaxResultsToShow ?? 6;

        public ResultsViewModel()
        {
            Results = new ResultCollection();
            BindingOperations.EnableCollectionSynchronization(Results, _collectionLock);
        }
        public ResultsViewModel(Settings settings) : this()
        {
            _settings = settings;
            _settings.PropertyChanged += (s, e) =>
            {
                if (e.PropertyName == nameof(_settings.MaxResultsToShow))
                {
                    Application.Current.Dispatcher.Invoke(() =>
                    {
                        OnPropertyChanged(nameof(MaxHeight));
                    });
                }
            };
        }

        #endregion

        #region Properties

        public int MaxHeight
        {
            get
            {
                return _settings.MaxResultsToShow * 75;
            }
        }
        public int SelectedIndex { get; set; }

        private ResultViewModel _selectedItem;
        public ResultViewModel SelectedItem
        {
            get { return _selectedItem; }
            set
            {
                //value can be null when selecting an item in a virtualized list
                if (value != null)
                {
                    if (_selectedItem != null)
                    {
                        _selectedItem.DeactivateContextButtons(ResultViewModel.ActivationType.Selection);
                    }

                    _selectedItem = value;
                    _selectedItem.ActivateContextButtons(ResultViewModel.ActivationType.Selection);
                }
                else
                {
                    _selectedItem = value;
                }
            }
        }



        public Thickness Margin { get; set; }
        public Visibility Visibility { get; set; } = Visibility.Hidden;

        #endregion

        #region Private Methods

        private int InsertIndexOf(int newScore, IList<ResultViewModel> list)
        {
            int index = 0;
            for (; index < list.Count; index++)
            {
                var result = list[index];
                if (newScore > result.Result.Score)
                {
                    break;
                }
            }
            return index;
        }

        private int NewIndex(int i)
        {
            var n = Results.Count;
            if (n > 0)
            {
                i = (n + i) % n;
                return i;
            }
            else
            {
                // SelectedIndex returns -1 if selection is empty.
                return -1;
            }
        }


        #endregion

        #region Public Methods

        public void SelectNextResult()
        {
            SelectedIndex = NewIndex(SelectedIndex + 1);
        }

        public void SelectPrevResult()
        {
            SelectedIndex = NewIndex(SelectedIndex - 1);
        }

        public void SelectNextPage()
        {
            SelectedIndex = NewIndex(SelectedIndex + _settings.MaxResultsToShow);
        }

        public void SelectPrevPage()
        {
            SelectedIndex = NewIndex(SelectedIndex - _settings.MaxResultsToShow);
        }

        public void SelectFirstResult()
        {
            SelectedIndex = NewIndex(0);
        }

        public void Clear()
        {
            Results.Clear();
        }

        public void RemoveResultsExcept(PluginMetadata metadata)
        {
            Results.RemoveAll(r => r.Result.PluginID != metadata.ID);
        }

        public void RemoveResultsFor(PluginMetadata metadata)
        {
            Results.RemoveAll(r => r.Result.PluginID == metadata.ID);
        }

        public void SelectNextTabItem()
        {
            //Do nothing if there is no selected item or we've selected the next context button
            if(!SelectedItem?.SelectNextContextButton() ?? true)
            {
                SelectNextResult();
            }
        }

        public void SelectPrevTabItem()
        {
            //Do nothing if there is no selected item or we've selected the previous context button
            if (!SelectedItem?.SelectPrevContextButton() ?? true)
            {
                //Tabbing backwards should highlight the last item of the previous row
                SelectPrevResult();
                SelectedItem.SelectLastContextButton();
            }
        }

        /// <summary>
        /// To avoid deadlock, this method should not called from main thread
        /// </summary>
        public void AddResults(List<Result> newRawResults, string resultId)
        {
            lock (_addResultsLock)
            {
                var newResults = NewResults(newRawResults, resultId);

                // update UI in one run, so it can avoid UI flickering
                Results.Update(newResults);

                if (Results.Count > 0)
                {
                    Margin = new Thickness { Top = 8 };
                    SelectedIndex = 0;
                }
                else
                {
                    Margin = new Thickness { Top = 0 };
                    Visibility = Visibility.Collapsed;
                }
            }
        }

        private List<ResultViewModel> NewResults(List<Result> newRawResults, string resultId)
        {
            var results = Results.ToList();
            var newResults = newRawResults.Select(r => new ResultViewModel(r)).ToList();
            var oldResults = results.Where(r => r.Result.PluginID == resultId).ToList();

            // Find the same results in A (old results) and B (new newResults)          
            var sameResults = oldResults
                                .Where(t1 => newResults.Any(x => x.Result.Equals(t1.Result)))
                                .ToList();

            // remove result of relative complement of B in A
            foreach (var result in oldResults.Except(sameResults))
            {
                results.Remove(result);
            }

            // update result with B's score and index position
            foreach (var sameResult in sameResults)
            {
                int oldIndex = results.IndexOf(sameResult);
                int oldScore = results[oldIndex].Result.Score;
                var newResult = newResults[newResults.IndexOf(sameResult)];
                int newScore = newResult.Result.Score;
                if (newScore != oldScore)
                {
                    var oldResult = results[oldIndex];

                    oldResult.Result.Score = newScore;
                    oldResult.Result.OriginQuery = newResult.Result.OriginQuery;

                    results.RemoveAt(oldIndex);
                    int newIndex = InsertIndexOf(newScore, results);
                    results.Insert(newIndex, oldResult);
                }
            }

            // insert result in relative complement of A in B
            foreach (var result in newResults.Except(sameResults))
            {
                int newIndex = InsertIndexOf(result.Result.Score, results);
                results.Insert(newIndex, result);
            }

            return results;
        }
        #endregion

        #region FormattedText Dependency Property
        public static readonly DependencyProperty FormattedTextProperty = DependencyProperty.RegisterAttached(
            "FormattedText",
            typeof(Inline),
            typeof(ResultsViewModel),
            new PropertyMetadata(null, FormattedTextPropertyChanged));

        public static void SetFormattedText(DependencyObject textBlock, IList<int> value)
        {
            textBlock.SetValue(FormattedTextProperty, value);
        }

        public static Inline GetFormattedText(DependencyObject textBlock)
        {
            return (Inline)textBlock.GetValue(FormattedTextProperty);
        }

        private static void FormattedTextPropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            var textBlock = d as TextBlock;
            if (textBlock == null) return;

            var inline = (Inline)e.NewValue;

            textBlock.Inlines.Clear();
            if (inline == null) return;

            textBlock.Inlines.Add(inline);
        }
        #endregion

        public class ResultCollection : ObservableCollection<ResultViewModel>
        {

            public void RemoveAll(Predicate<ResultViewModel> predicate)
            {
                CheckReentrancy();

                for (int i = Count - 1; i >= 0; i--)
                {
                    if (predicate(this[i]))
                    {
                        RemoveAt(i);
                    }
                }
            }

            /// <summary>
            /// Update the results collection with new results, try to keep identical results
            /// </summary>
            /// <param name="newItems"></param>
            public void Update(List<ResultViewModel> newItems)
            {
                int newCount = newItems.Count;
                int oldCount = Items.Count;
                int location = newCount > oldCount ? oldCount : newCount;

                for (int i = 0; i < location; i++)
                {
                    ResultViewModel oldResult = this[i];
                    ResultViewModel newResult = newItems[i];
                    if (!oldResult.Equals(newResult))
                    { // result is not the same update it in the current index
                        this[i] = newResult;
                    }
                    else if (oldResult.Result.Score != newResult.Result.Score)
                    {
                        this[i].Result.Score = newResult.Result.Score;
                    }
                }


                if (newCount >= oldCount)
                {
                    for (int i = oldCount; i < newCount; i++)
                    {
                        Add(newItems[i]);
                    }
                }
                else
                {
                    for (int i = oldCount - 1; i >= newCount; i--)
                    {
                        RemoveAt(i);
                    }
                }
            }
        }
    }
}