using Livet;
using Livet.EventListeners;
using System.ComponentModel;
using System.Runtime.CompilerServices;

namespace AeroBeatTools.Mvvm
{
    public abstract class BindableViewModel : ViewModel
    {
        protected virtual bool SetProperty<T>(ref T storage, T value, [CallerMemberName] string propertyName = null)
        {
            if (Equals(storage, value)) return false;

            storage = value;
            RaisePropertyChanged(propertyName);
            return true;
        }
    }

    public abstract class BindableViewModel<T> : BindableViewModel where T : INotifyPropertyChanged
    {
        protected T Model;

        public BindableViewModel(T model, bool createDefaultEventListener = true)
        {
            Model = model;
            if (createDefaultEventListener)
                CompositeDisposable.Add(new PropertyChangedEventListener(Model, (sender, e) => RaisePropertyChanged(e.PropertyName)));
        }
    }
}
