// Function to subtract two integers
int sub(int i, int j)
{
    // If i is greater than j, return i - j
    if (i > j)
        return (i - j);
    else
        // Otherwise, return j - i (this ensures the result is always non-negative)
        return (j - i);
}
