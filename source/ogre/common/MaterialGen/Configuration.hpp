namespace sh
{
	/**
	 * @brief
	 * a \a Configuration allows you to create a specialized set of shaders for specific purposes. \n
	 * for example, you might want to not include shadows in reflection targets. \n
	 * a \a Configuration can override or unset any property in a shader
	 */
	class Configuration : public PropertySetGet
	{
	};
}
