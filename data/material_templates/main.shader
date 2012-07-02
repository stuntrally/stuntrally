
#ifdef SH_VERTEX_SHADER


void main(
						float4 position : POSITION,

                            float3 normal   : NORMAL,



                        out float4 oPosition : POSITION,

                        out float3 objectPos : TEXCOORD0,

                        out float3 oNormal   : TEXCOORD1,



                    uniform float4x4 modelViewProj @shAutoConstant(modelViewProj, worldviewproj_matrix)

                )

{

  oPosition = mul(modelViewProj, position);

  objectPos = position.xyz;

  oNormal = normal;

}


#else


void main(float4 position  : TEXCOORD0,

          float3 normal    : TEXCOORD1,



              out float4 color     : COLOR,



              uniform float3 globalAmbient, @shAutoConstant(globalAmbient, ambient_light_colour)

              uniform float3 lightColor, @shAutoConstant(lightColor, light_diffuse_colour, 0)

              uniform float3 lightPosition, @shAutoConstant(lightPosition, light_position_object_space, 0)

			  uniform float3 materialDiffuse @shAutoConstant(materialDiffuse, surface_diffuse_colour)

)

{

  float3 P = position.xyz;

  float3 N = normalize(normal);


  // Compute the ambient term

  float3 ambient = globalAmbient;



  // Compute the diffuse term

  float3 L = normalize(lightPosition - P);

  float diffuseLight = max(dot(N, L), 0);

  float3 diffuse = materialDiffuse * lightColor * diffuseLight;


  color.xyz = ambient + diffuse;

  color.w = 1;

}

#endif
