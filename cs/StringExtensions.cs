namespace ExtensionMethods
{
    public static class StringExtensions
    {
        public static string ReplaceLastOf(this string original, string replacee, string replacer)
        {
            int place = original.LastIndexOf(replacee);

            if (place < 0)
                return original;

            return original.Remove(place, replacee.Length).Insert(place, replacer);
        }
    }
}