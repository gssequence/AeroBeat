using Livet;
using System.Runtime.CompilerServices;

namespace AeroBeatTools.Mvvm
{
    public abstract class BindableModel : NotificationObject
    {
        protected virtual bool SetProperty<T>(ref T storage, T value, [CallerMemberName] string propertyName = null)
        {
            if (Equals(storage, value)) return false;

            storage = value;
            RaisePropertyChanged(propertyName);
            return true;
        }
    }
}
