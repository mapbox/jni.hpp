class HighLevelBinding {
    public native void greet(String args);
    public native void greet(int args);
    public static native void greet(double args);
    public int quadruple(int num) {
        return num * 4;
    }
}

public class Binding {
    public static void main(String[] args) {
        System.loadLibrary("binding");
        new HighLevelBinding().greet("test");
        new HighLevelBinding().greet(4);
        HighLevelBinding.greet(3.14);
    }
}
