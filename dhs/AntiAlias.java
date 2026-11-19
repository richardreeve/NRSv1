package btn.stuff ;

import java.awt.*;

/**
AntiAlias<br>
Static (toolkit) class to enable and disable antialiasing for a Graphics2D object.<br>
<br>
Version 0.5<br>
by Steven de Jong<br>
20/Sep/2001
*/
public class AntiAlias
{
	/**
	Enable antialiasing for the given Graphics2D object.
	@param g2 The Graphics2D object.
	*/
	public static void enableFor(Graphics2D g2)
	{
		RenderingHints antiAlias = new RenderingHints(
			RenderingHints.KEY_ANTIALIASING,
			RenderingHints.VALUE_ANTIALIAS_ON
		);
		antiAlias.put(
			RenderingHints.KEY_TEXT_ANTIALIASING,
			RenderingHints.VALUE_TEXT_ANTIALIAS_OFF);
		antiAlias.put(
			RenderingHints.KEY_RENDERING,
			RenderingHints.VALUE_RENDER_QUALITY);
		g2.addRenderingHints(antiAlias);
	}

	/**
	Disable antialiasing for the given Graphics2D object.
	@param g2 The Graphics2D object.
	*/
	public static void disableFor(Graphics2D g2)
	{
		RenderingHints antiAlias = new RenderingHints(
			RenderingHints.KEY_ANTIALIASING,
			RenderingHints.VALUE_ANTIALIAS_OFF
		);
		antiAlias.put(
			RenderingHints.KEY_TEXT_ANTIALIASING,
			RenderingHints.VALUE_TEXT_ANTIALIAS_OFF);
		antiAlias.put(
			RenderingHints.KEY_RENDERING,
			RenderingHints.VALUE_RENDER_SPEED);
		g2.addRenderingHints(antiAlias);
	}
}
