void main()
{
        vec3 paramDiffuse = vec3(1,1,1);
        vec3 paramEmissive = vec3(0,0,0);
        vec3 paramNormals = vec3(0,0,1);
        float paramSpecular = 0.5;
        float paramSpecularPower = 0.5;
        vec2 projectedCoord = vec2(0,0);
        getProjectedCoord(projectedCoord);

        computeShaderParameters(paramDiffuse, paramNormals, paramSpecular, paramSpecularPower, paramEmissive, projectedCoord);

        outColor = vec4( paramDiffuse, paramSpecular );
        outHighValues = vec4(paramEmissive, 1.0);

        vec3 bumpNormal = paramNormals;
        bumpNormal = normalize(bumpNormal * 2.0 - 1.0);
        bumpNormal = normalize(In.TBN * bumpNormal);
        outNormal = vec4( bumpNormal*0.5+0.5, paramSpecularPower/100.0 );

        outPositions = vec4(In.Position, 1.0);
}
