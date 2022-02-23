using System;
using System.Threading.Tasks;
using Windows.Management.Policies;
using Windows.UI;
using Windows.UI.Core;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Media;

namespace UserPolicy
{
    public sealed partial class MainPage : Page
    {
        public int RefreshedCount
        {
            get { return (int)GetValue(RefreshedCountProperty); }
            set { SetValue(RefreshedCountProperty, value); }
        }

        public string GraphingModeState
        {
            get { return (string)GetValue(GraphingModeStateProperty); }
            set { SetValue(GraphingModeStateProperty, value); }
        }

        public static readonly DependencyProperty RefreshedCountProperty =
            DependencyProperty.Register("RefreshedCount", typeof(int), typeof(MainPage), new PropertyMetadata(0));

        public static readonly DependencyProperty GraphingModeStateProperty =
            DependencyProperty.Register("GraphingModeState", typeof(string), typeof(MainPage), new PropertyMetadata("Unknown"));

        public MainPage()
        {
            InitializeComponent();
            _ = Dispatcher.RunAsync(CoreDispatcherPriority.High, async () =>
            {
                var usr = ((App)Application.Current).CurrentUser;
                while (true)
                {
                    var policy = NamedPolicy.GetPolicyFromPathForUser(usr, "Education", "AllowGraphingCalculator");
                    GraphingModeState = policy.GetBoolean() ? "Enabled" : "Disabled";
                    ++RefreshedCount;
                    await Task.Delay(TimeSpan.FromSeconds(0.5));
                }
            });
        }

        public static Brush GreenIf(string state)
        {
            return new SolidColorBrush(state == "Enabled" ? Colors.Green : Colors.Red);
        }
    }
}
