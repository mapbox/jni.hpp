class Calculator {
    public Calculator() {
        initialize();
    }

    private long peer;
    protected native void initialize();
    protected native void finalize() throws Throwable;

    public native long add(long a, long b);
    public native long subtract(long a, long b);
}

public class NativePeer {
    public static void main(String[] args) {
        System.loadLibrary("peer");

        Calculator calculator = new Calculator();
        System.out.println("2 + 2 = " + calculator.add(2, 2));
        System.out.println("8 - 4 = " + calculator.subtract(8, 4));

        // You wouldn't normally use this; it's here to show that the native finalizer does get executed.
        System.runFinalizersOnExit(true);
    }
}
